#pragma once
#include <kengine/ExecutorService.hpp>
#include <unordered_map>


template <typename V, typename K>
class AsyncAssetCache {
private:
    ExecutorService& workerPool;

    std::unordered_map<size_t, std::unique_ptr<V>> cache;
    std::unordered_map<size_t, std::future<V*>> backgroundTasks;
    std::mutex lock;

protected:
    virtual std::unique_ptr<V> create(std::shared_ptr<K> keyObj) = 0;

public:
    AsyncAssetCache(ExecutorService& workerPool) : workerPool(workerPool) {}

    V* unsafeAdd(K& keyObj, std::unique_ptr<V>&& value) {
        size_t key = keyObj.hashCode();
        std::lock_guard<std::mutex> lock(this->lock);
        auto& asset = cache[key] = std::move(value);
        return asset.get();
    }

    std::unique_ptr<V> remove(K& keyObj) {
        std::lock_guard<std::mutex> lock(this->lock);
        size_t key = keyObj.hashCode();
        auto it = cache.find(key);
        if (it == cache.end())
            return nullptr;

        auto res = std::move(it->second);

        cache.erase(it);

        return res;
    }

    std::future<V*> get(std::shared_ptr<K> keyObj) {
        std::lock_guard<std::mutex> lock(this->lock);
        size_t key = keyObj->hashCode();
        auto cacheIt = cache.find(key);
        if (cacheIt != cache.end())
            return std::async(std::launch::deferred, [asset = cacheIt->second.get()]() { return asset; });

        auto taskIt = backgroundTasks.find(key);
        if (taskIt != backgroundTasks.end())
            return taskIt->second;

        auto task = workerPool.submit([this, key, keyObj]() {
            auto asset = this->create(keyObj);

            std::lock_guard<std::mutex> lock(this->lock);
            auto& storedAsset = this->cache[key] = std::move(asset);
            this->backgroundTasks.erase(key);

            return storedAsset.get();
            });

        backgroundTasks[key] = task;

        return task;
    }
};