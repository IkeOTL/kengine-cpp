#pragma once
#include "ExecutorService.hpp"
#include <kengine/Logger.hpp>

#include <functional>
#include <future>
#include <optional>

class World;

class BaseDeferredJob {
public:
    virtual ~BaseDeferredJob() = default;
    virtual bool isDone() const = 0;
    virtual void executeThenFunc(World& world) = 0;
};

template<typename T, typename Func>
class DeferredJob : public BaseDeferredJob {
private:
    std::shared_future<T> future;
    Func thenFunc;

public:
    DeferredJob(std::shared_future<T> future, Func&& thenFunc)
        : future(std::move(future)), thenFunc(std::move(thenFunc)) {}

    bool isDone() const override {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void executeThenFunc(World& world) override {
        try {
            if constexpr (std::is_void_v<T>) {
                future.get();
                thenFunc(world);
            }
            else {
                auto result = future.get();
                thenFunc(world, result);
            }
        }
        catch (const std::exception& e) {
            KE_LOG_ERROR(std::format("Follow-up function failed: {}", e.what()));
        }
    }
};

class DeferredJobManager {
private:
    using BaseJobPtr = std::unique_ptr<BaseDeferredJob>;
    std::deque<BaseJobPtr> newJobs;
    std::deque<BaseJobPtr> jobs;
    std::mutex newJobLock;

public:

    inline static std::unique_ptr<DeferredJobManager> create() {
        return std::make_unique<DeferredJobManager>();
    }

    template<typename T, typename Func>
    void submit(World& world, std::shared_future<T> task, Func&& thenFunc) {
        if constexpr (std::is_void_v<T>)
            static_assert(std::is_invocable_v<Func, World&>, "`thenFunc` is missing `World&` parameter.");
        else
            static_assert(std::is_invocable_v<Func, World&, T>, "`thenFunc` is missing `World&` and `T` parameter.");

        std::lock_guard<std::mutex> lock(newJobLock);
        newJobs.emplace_back(
            std::make_unique<DeferredJob<T, Func>>(std::move(task), std::forward<Func>(thenFunc))
        );
    }

    void process(World& world) {
        if (newJobs.empty() && jobs.empty())
            return;

        if (!newJobs.empty()) {
            std::lock_guard<std::mutex> lock(newJobLock);

            jobs.insert(jobs.end(),
                std::make_move_iterator(newJobs.begin()),
                std::make_move_iterator(newJobs.end()));
            newJobs.clear();
        }

        auto it = jobs.begin();
        while (it != jobs.end()) {
            auto& ptr = *it;

            if (!ptr->isDone()) {
                ++it;
                continue;
            }

            ptr->executeThenFunc(world);
            it = jobs.erase(it);
        }
    }
};