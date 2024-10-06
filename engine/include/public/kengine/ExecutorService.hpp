#pragma once
#include <thread>
#include <queue>
#include <future>
#include <condition_variable>
#include <iostream>
#include <optional>

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
                        std::cerr << "Exception in worker: " << e.what() << std::endl;
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

//template<typename R>
//class YeildingTask {
//private:
//    ExecutorService& threadPool;
//    std::shared_ptr<std::promise<R>> promise;
//
//public:
//    YeildingTask(ExecutorService& threadPool)
//        : threadPool(threadPool) {}
//
//    virtual void run() = 0;
//
//protected:
//    void setResult(R res) {
//        promise->set_value(res);
//    }
//
//    void yield() {
//        threadPool.submit([this]() { this->run(); });
//    }
//};
//
//template<typename R, typename N>
//class YeildingMultiTask : YeildingTask {
//private:
//    enum class TaskState {
//        Start,
//        Running,
//        Done
//    };
//
//    std::array<R, N> tasks;
//    TaskState state;
//
//public:
//    YeildingMultiTask(ExecutorService& threadPool)
//        : YeildingTask(threadPool) {}
//
//    void run() {
//        switch (state) {
//        case TaskState::Start:
//            for (size_t i = 0; i < length; i++) {
//                tasks.push_back([]() {asdasdasd})
//            }
//
//            state = TaskState::Running;
//
//            yeild();
//            break;
//        case TaskState::Running:
//            if (areTasksDone(tasks))
//                setResult(asdasdasd);
//            break;
//        }
//    }
//};