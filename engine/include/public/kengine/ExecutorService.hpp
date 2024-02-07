#pragma once
#include <thread>
#include <queue>
#include <future>
#include <condition_variable>

class ExecutorService {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;

public:
    ExecutorService(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock,
                            [this] {
                                return this->stop || !this->tasks.empty();
                            });

                        if (this->stop && this->tasks.empty())
                            return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
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

    void execute(const std::function<void()>& task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stop)
                throw std::runtime_error("ThreadPool is stopping");

            tasks.emplace(task);
        }

        condition.notify_one();
    }

    /// <summary>
    /// Submits to pool and returns a single use future.
    /// </summary>
    template<typename F>
    auto submit(F&& f) -> std::future<decltype(f())> {
        using ReturnType = decltype(f());
        using FutureType = std::future<ReturnType>;

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