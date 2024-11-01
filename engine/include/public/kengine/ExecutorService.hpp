#pragma once
#include <kengine/Logger.hpp>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#include <thread>
#include <queue>
#include <future>
#include <condition_variable>
#include <iostream>
#include <optional>
#include <memory>

namespace ke {
    class ExecutorService {
    private:
        tf::Executor executor;

    public:
        ExecutorService(size_t numThreads, const std::optional<std::function<void()>> onThreadStartup = std::nullopt)
            : executor(numThreads) {
            if (onThreadStartup) {
                tf::Taskflow taskflow;
                int num_threads = executor.num_workers();
                taskflow.for_each_index(0, num_threads, 1, [&, onThreadStartup](int i) {
                    KE_LOG_DEBUG(std::format("Initialized pool thread: {}", i));
                    (*onThreadStartup)();
                });
                this->executor.run(taskflow).wait();
            }
        }

        ~ExecutorService() = default;

        /// @brief grabs the Taskflow executor for more involved task dispatching
        tf::Executor& getExectuor() {
            return executor;
        }

        void execute(std::function<void()>&& task) {
            executor.silent_async(task);
        }

        /// @brief Submits to pool and returns a single use future.
        template<typename F>
        auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
            using ReturnType = std::invoke_result_t<F>;
            return executor.async(std::forward<F>(f));
        }

        /// @brief Submits to pool and returns a shared future.
        template<typename F>
        auto submitShared(F&& f) -> std::shared_future<decltype(f())> {
            auto future = submit(std::forward<F>(f));
            return future.share();
        }
    };
} // namespace ke