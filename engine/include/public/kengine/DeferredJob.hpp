#pragma once
#include "ExecutorService.hpp"

#include <functional>
#include <future>
#include <optional>

class World;

template<typename T>
class DeferredJob {
public:
    using TaskFunc = std::function<T(World&)>;
    using ReadyFunc = std::function<void(World&, T)>;

private:
    TaskFunc task;
    std::future<T> future;
    ReadyFunc ready;

public:
    DeferredJob(TaskFunc&& task, ReadyFunc&& ready)
        : task(std::move(task)), ready(std::move(ready)) {}

    DeferredJob(TaskFunc&& task)
        : task(std::move(task)), ready(nullptr)) {}

    TaskFunc& getTask() const {
        return task;
    }

    std::future<T>& getFuture() {
        return future;
    }

    void setFuture(std::future<T>&& fut) {
        future = std::move(fut);
    }

    ReadyFunc* getReadyFunc() const {
        return  ready ? &ready : nullptr;;
    }
};

class DeferredJobManager {
private:
    ExecutorService& workerPool;
    std::vector<std::unique_ptr<DeferredJob<void>>> newJobs;
    std::vector<std::unique_ptr<DeferredJob<void>>> jobs;
    std::mutex newJobLock;

public:
    explicit DeferredJobManager(ExecutorService& workerPool)
        : workerPool(workerPool) {}

    template<typename T>
    void job(World& world, std::unique_ptr<DeferredJob<T>> j) {
        std::lock_guard<std::mutex> lock(newJobLock);
        auto future = workerPool.submitShared([&world, task& = j->getTask()]() {
            return task(world);
            });
        j->setFuture(std::move(future));
        newJobs.push_back(std::move(j));
    }

    void process(World& world) {
        {
            std::lock_guard<std::mutex> lock(newJobLock);
            std::move(std::make_move_iterator(newJobs.begin()), std::make_move_iterator(newJobs.end()), std::back_inserter(jobs));
            newJobs.clear();
        }

        auto it = jobs.begin();
        while (it != jobs.end()) {
            auto& job = *it;

            auto& future = job->getFuture();
            if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                ++it;
                continue;
            }

            auto stepTwo = job->getReadyFunc();
            if (!stepTwo) {
                it = jobs.erase(it);
                continue;
            }

            auto res = future.get();
            stepTwo(world, res);
            it = jobs.erase(it);
        }
    }
};

