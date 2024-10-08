#pragma once
#include <kengine/Logger.hpp>
#include <thread>
#include <queue>
#include <future>
#include <condition_variable>
#include <iostream>
#include <optional>
#include <memory>

class ExecutorService {
private:
    struct WorkerQueue {
        std::deque<std::function<void()>> tasks;
        std::mutex mutex;
        std::atomic<bool> isEmpty = true;
    };

    std::atomic<uint32_t> totalTaskCount = 0;
    std::vector<std::thread> workers;
    std::vector<std::unique_ptr<WorkerQueue>> workerQueues;
    std::mutex mainMutex{};
    std::condition_variable condition;
    std::atomic<bool> stop = false;
    std::atomic<uint32_t> targetIdx = 0;

private:
    bool fetchTask(uint32_t tIdx, std::function<void()>& task) {
        auto& q = workerQueues[tIdx];
        std::lock_guard<std::mutex> lock(q->mutex);

        if (q->tasks.empty()) {
            q->isEmpty.store(true);
            return false;
        }

        task = std::move(q->tasks.front());
        q->tasks.pop_front();
        totalTaskCount.fetch_sub(1, std::memory_order_relaxed);
        q->isEmpty.store(q->tasks.empty());

        return true;
    }

    bool stealTask(uint32_t tIdx, uint32_t numThreads, std::function<void()>& task) {
        for (auto i = 0; i < numThreads; ++i) {
            // do our current queue last
            auto victimIdx = (tIdx + i) % numThreads;

            if (victimIdx == tIdx)
                continue;

            {
                auto& q = workerQueues[victimIdx];
                std::lock_guard<std::mutex> lock(q->mutex);

                if (q->tasks.empty()) {
                    q->isEmpty.store(true);
                    return false;
                }

                KE_LOG_INFO(std::format("[Thread: {}] stole a task from {}.", tIdx, victimIdx));
                task = std::move(q->tasks.back());
                q->tasks.pop_back();
                totalTaskCount.fetch_sub(1, std::memory_order_relaxed);
                q->isEmpty.store(q->tasks.empty());

                return true;
            }
        }

        return false;
    }

    template<typename R, typename F>
    void yield(F task, std::shared_ptr<std::promise<R>> promise) {
        execute([this, task = std::move(task), promise]() mutable {
            try {
                if (!task(*promise))
                    this->yield(std::move(task), promise);
            }
            catch (...) {
                try {
                    promise->set_exception(std::current_exception());
                }
                catch (...) {}
            }
            });
    }

    bool allQueuesEmpty() const {
        return totalTaskCount.load(std::memory_order_relaxed) == 0;
    }

public:
    ExecutorService(size_t numThreads, const std::optional<std::function<void()>> onThreadStartup = std::nullopt) {
        workers.reserve(numThreads);
        workerQueues.reserve(numThreads);

        for (size_t i = 0; i < numThreads; ++i) {
            workerQueues.emplace_back(std::make_unique<WorkerQueue>());

            workers.emplace_back([this, numThreads, tIdx = i, onThreadStartup] {
                if (onThreadStartup)
                    (*onThreadStartup)();

                while (true) {
                    std::function<void()> task;

                    // try to work on own task
                    if (fetchTask(tIdx, task) || stealTask(tIdx, numThreads, task)) {
                        try {
                            task();
                        }
                        catch (const std::exception& e) {
                            KE_LOG_ERROR(std::format("Exception in worker: {}", e.what()));
                        }
                        catch (...) {
                            KE_LOG_ERROR("Unknown exception in worker thread.");
                        }

                        continue;
                    }

                    {
                        std::unique_lock<std::mutex> lock(this->mainMutex);
                        this->condition.wait(lock,
                            [this, tIdx] {
                                return this->stop.load()
                                    || !this->workerQueues[tIdx]->isEmpty.load()
                                    || !this->allQueuesEmpty();
                            });

                        if (this->stop && this->allQueuesEmpty())
                            return;
                    }
                }
                });
        }
    }

    ~ExecutorService() {
        {
            std::lock_guard<std::mutex> lock(mainMutex);
            stop = true;
        }

        condition.notify_all();

        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    /// <summary>
    /// executes a function. the executed function returns another function that will keep checking a condition, 
    /// if it returns false the task is submitted back into the queue allowing other work to progress, 
    /// then it will try again until true is returned.
    /// Use case: loading lots of assets and executing an operation once all asset futures are ready.
    /// </summary>
    template<typename R, typename F>
    std::shared_future<R> submitYielding(F&& initTask) {
        auto promisePtr = std::make_shared<std::promise<R>>();
        auto future = promisePtr->get_future().share();

        // Initialize the yielding task with the ExecutorService reference
        auto stepTask = initTask(*this);

        // Ensure that the initializer returns a valid step function
        static_assert(
            std::is_invocable_r_v<bool, decltype(stepTask), std::promise<R>&>,
            "The step function must be callable with std::promise<R>& and return bool."
            );

        yield<R>(std::move(stepTask), promisePtr);

        return future;
    }

    void execute(std::function<void()>&& task) {
        auto tIdx = targetIdx.fetch_add(1) % workerQueues.size();

        {
            std::lock_guard<std::mutex> lock(workerQueues[tIdx]->mutex);

            if (stop) {
                KE_LOG_WARN("Thread pool is stopping.");
                return;
            }

            workerQueues[tIdx]->tasks.emplace_back(std::move(task));
            totalTaskCount.fetch_add(1, std::memory_order_relaxed);
            workerQueues[tIdx]->isEmpty.store(false);
        }

        // need to determine which works best
        condition.notify_all();
        //condition.notify_one();
    }

    /// <summary>
    /// Submits to pool and returns a single use future.
    /// </summary>
    template<typename F>
    auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
        auto tIdx = targetIdx.fetch_add(1) % workerQueues.size();

        using ReturnType = std::invoke_result_t<F>;

        if (stop) {
            std::promise<ReturnType> promise;
            promise.set_exception(std::make_exception_ptr(std::runtime_error("Thread pool is stopping.")));
            return promise.get_future();
        }

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));

        {
            std::lock_guard<std::mutex> lock(workerQueues[tIdx]->mutex);

            workerQueues[tIdx]->tasks.emplace_back([task]() { (*task)(); });
            totalTaskCount.fetch_add(1, std::memory_order_relaxed);
            workerQueues[tIdx]->isEmpty.store(false);
        }

        // need to determine which works best
        condition.notify_all();
        //condition.notify_one();

        return task->get_future();
    }

    /// <summary>
    /// Submits to pool and returns a shared future.
    /// </summary>
    template<typename F>
    auto submitShared(F&& f) -> std::shared_future<decltype(f())> {
        auto future = submit(std::forward<F>(f));
        return future.share();
    }
};