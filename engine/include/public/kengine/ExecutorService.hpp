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
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex{};
    std::condition_variable condition;
    bool stop = false;

public:
    ExecutorService(size_t numThreads, const std::optional<std::function<void()>> onThreadStartup = std::nullopt) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, onThreadStartup] {
                if (onThreadStartup)
                    (*onThreadStartup)();

                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

                        if (this->stop && this->tasks.empty())
                            return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    try {
                        task();
                    }
                    catch (const std::exception& e) {
                        KE_LOG_ERROR(std::format("Exception in worker: {}", e.what()));
                    }
                }
                });
        }
    }

    ~ExecutorService() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }

        condition.notify_all();

        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    template<typename R, typename F>
    void yield(F&& task, std::shared_ptr<std::promise<R>> promise) {
        execute([this, task = std::forward<F>(task), promise]() mutable {
            try {
                if (!task(*promise))
                    this->yield(std::forward<F>(task), promise);
            }
            catch (...) {
                try {
                    promise->set_exception(std::current_exception());
                }
                catch (...) {}
            }
            });
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
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stop)
                throw std::runtime_error("ThreadPool is stopping");

            tasks.emplace(std::move(task));
        }

        condition.notify_one();
    }

    /// <summary>
    /// Submits to pool and returns a single use future.
    /// </summary>
    template<typename F>
    auto submit(F&& f) -> std::future<typename std::invoke_result_t<F>> {
        using ReturnType = typename std::invoke_result_t<F>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stop)
                throw std::runtime_error("ThreadPool is stopping");

            tasks.emplace([task]() { (*task)(); });
        }

        condition.notify_one();

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