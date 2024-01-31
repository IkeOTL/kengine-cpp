#pragma once
#include <kengine/ExecutorService.hpp>
#include <unordered_map>

template <typename T, typename C>
class AsyncAssetCache {

private:
    const ExecutorService& workerPool;

    std::unordered_map<C, std::unique_ptr<T>> cache;
    std::unordered_map<C, std::future<T*>> backgroundTasks;
    std::mutex lock;

public:
    AsyncAssetCache(ExecutorService& workerPool) :workerPool(workerPool) {}

    virtual std::unique_ptr<T> create(C key) = 0;

    T* unsafeAdd(C key, std::unique_ptr<T>&& value) {
        std::unique_lock<std::mutex> lock(this->lock);
        auto& asset = cache[key] = std::move(value);
        return asset.get();
    }

    std::unique_ptr<T> remove(C key) {
        std::unique_lock<std::mutex> lock(this->lock);

        auto it = cache.find(key);
        if (it == cache.end())
            return nullptr;

        auto res = std::move(it->second);

        cache.erase(it);

        return res;
    }

    std::future<T*> get(C key) {
        std::unique_lock<std::mutex> lock(this->lock);

        auto cacheIt = cache.find(key);
        if (cacheIt != cache.end()) {
            std::promise<T*> p;
            p.set_value(cacheIt->second.get());
            return p.get_future();
        }

        auto taskIt = backgroundTasks.find(key);
        if (taskIt != map.end())
            return taskIt->second;

        auto task = workerPool.submit([this]() {
            auto asset = this->create(key);

            std::unique_lock<std::mutex> lock(this->lock);
            auto& storedAsset = this->cache[key] = std::move(asset);
            this->backgroundTasks.erase(key);

            return storedAsset.get();
            });

        backgroundTasks[key] = task;

        return task;
    }
};