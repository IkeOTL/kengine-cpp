#pragma once
#include <kengine/ExecutorService.hpp>
#include <unordered_map>
#include <functional>
#include <shared_mutex>

template <typename V, typename K>
class AsyncAssetCache {
private:
    ExecutorService& workerPool;

    // maybe change to shared pointers? profile later
    std::unordered_map<size_t, std::unique_ptr<V>> cache;
    std::unordered_map<size_t, std::shared_future<V*>> backgroundTasks;
    std::shared_mutex lock;

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

    V& get(std::shared_ptr<K> keyObj) {
        size_t key = keyObj->hashCode();

        // quick check with shared read lock
        {
            std::shared_lock<std::shared_mutex> lock(this->lock);

            // already loaded
            auto cacheIt = cache.find(key);
            if (cacheIt != cache.end())
                return *(cacheIt->second.get());

            // currently loading
            auto taskIt = backgroundTasks.find(key);
            if (taskIt != backgroundTasks.end()) {
                auto task = taskIt->second;
                lock.unlock(); // dont deadlock, the task is expected to acquire a write lock
                return *(taskIt->second.get());
            }
        }

        // write
        {
            std::unique_lock<std::shared_mutex> lock(this->lock);

            // already loaded
            auto cacheIt = cache.find(key);
            if (cacheIt != cache.end())
                return *(cacheIt->second.get());

            // double check
            auto taskIt = backgroundTasks.find(key);
            if (taskIt != backgroundTasks.end()) {
                auto task = taskIt->second;
                lock.unlock(); // dont deadlock, the task is expected to acquire a write lock
                return *(taskIt->second.get());
            }

            std::promise<V*> promise;

            backgroundTasks[key] = promise.get_future().share();

            lock.unlock();
            auto asset = this->create(keyObj);

            {
                lock.lock();
                auto& storedAsset = this->cache[key] = std::move(asset);
                promise.set_value(storedAsset.get());
                this->backgroundTasks.erase(key);
            }

            return *(storedAsset.get());
        }
    }

    // possible improvement: have multiple semaphore for the different phases so we dont block the entire section
    std::shared_future<V*> getAsync(std::shared_ptr<K> keyObj) {
        size_t key = keyObj->hashCode();

        // quick check with shared read lock
        {
            std::shared_lock<std::shared_mutex> lock(this->lock);

            // already loaded
            auto cacheIt = cache.find(key);
            if (cacheIt != cache.end()) {
                std::promise<V*> promise;
                promise.set_value(cacheIt->second.get());
                return promise.get_future().share();
            }

            // currently loading
            auto taskIt = backgroundTasks.find(key);
            if (taskIt != backgroundTasks.end())
                return taskIt->second;
        }

        // write
        {
            std::lock_guard<std::shared_mutex> lock(this->lock);

            // already loaded
            auto cacheIt = cache.find(key);
            if (cacheIt != cache.end()) {
                std::promise<V*> promise;
                promise.set_value(cacheIt->second.get());
                return promise.get_future().share();
            }

            // double check
            auto taskIt = backgroundTasks.find(key);
            if (taskIt != backgroundTasks.end())
                return taskIt->second;

            auto task = workerPool.submitShared([this, key, keyObj]() {
                // need to handle exception to propagate to caller
                auto asset = this->create(keyObj);

                {
                    std::lock_guard<std::shared_mutex> lock(this->lock);
                    auto& storedAsset = this->cache[key] = std::move(asset);
                    this->backgroundTasks.erase(key);

                    return storedAsset.get();
                }
                });

            backgroundTasks[key] = task;

            return task;
        }
    }
};