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
        std::lock_guard<std::mutex> lock(this->lock);
        auto& asset = cache[key] = std::move(value);
        return asset.get();
    }

    std::unique_ptr<T> remove(C key) {
        std::lock_guard<std::mutex> lock(this->lock);

        auto it = cache.find(key);
        if (it == cache.end())
            return nullptr;

        auto res = std::move(it->second);

        cache.erase(it);

        return res;
    }

    std::future<T*> get(C key) {
        std::lock_guard<std::mutex> lock(this->lock);

        auto cacheIt = cache.find(key);
        if (cacheIt != cache.end())
            return std::async(std::launch::deferred, [asset = cacheIt->second.get()]() { return asset; });

        auto taskIt = backgroundTasks.find(key);
        if (taskIt != backgroundTasks.end())
            return taskIt->second;

        auto task = workerPool.submit([this, key]() {
            auto asset = this->create(key);

            std::lock_guard<std::mutex> lock(this->lock);
            auto& storedAsset = this->cache[key] = std::move(asset);
            this->backgroundTasks.erase(key);

            return storedAsset.get();
            });

        backgroundTasks[key] = task;

        return task;
    }
};