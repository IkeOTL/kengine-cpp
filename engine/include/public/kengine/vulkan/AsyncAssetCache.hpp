#pragma once
#include <kengine/ExecutorService.hpp>
#include <kengine/Hashable.hpp>
#include <unordered_map>
#include <functional>
#include <shared_mutex>
#include <optional>
#include <tracy/Tracy.hpp>

namespace ke {
    /// <summary>
    /// Makes checking futures are done easier, also optimizes not needing to create dummy futures when asset is already cached
    /// </summary>
    template <typename V>
    class AsyncCacheTask {
    private:
        std::optional<std::shared_future<V>> task = std::nullopt;
        V result = nullptr;

    public:
        AsyncCacheTask(std::shared_future<V> future)
            : task(std::move(future)) {}
        AsyncCacheTask(V result)
            : result(result) {}

        bool isDone() const {
            if (task.has_value())
                return task.value().wait_for(std::chrono::seconds(0)) == std::future_status::ready;

            // check to avoid awaiting forever
            if (!result)
                throw std::runtime_error("Must either have a future, or a result.");

            return !!result;
        }

        V get() {
            if (task.has_value())
                return task.value().get();

            return result;
        }
    };

    template <typename V, typename K>
    class AsyncAssetCache {
    private:
        static_assert(std::is_base_of<Hashable, K>::value, "K must be derived from Hashable");

        ExecutorService& workerPool;

        // maybe change to shared pointers? profile later
        std::unordered_map<size_t, std::unique_ptr<V>> cache{};
        std::unordered_map<size_t, std::shared_future<V*>> backgroundTasks{};
        std::shared_mutex lock{};

    protected:
        virtual std::unique_ptr<V> create(std::shared_ptr<K> keyObj) = 0;

    public:
        AsyncAssetCache(ExecutorService& workerPool)
            : workerPool(workerPool) {}

        V* unsafeAdd(K& keyObj, std::unique_ptr<V>&& value) {
            size_t key = keyObj.hashCode();
            std::lock_guard<std::shared_mutex> lock(this->lock);
            auto& asset = cache[key] = std::move(value);
            return asset.get();
        }

        std::unique_ptr<V> remove(K& keyObj) {
            std::lock_guard<std::shared_mutex> lock(this->lock);
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

                lock.unlock(); // allow other cache requests to be fulfilled
                auto asset = this->create(keyObj);

                lock.lock();
                auto& storedAsset = this->cache[key] = std::move(asset);
                promise.set_value(storedAsset.get());
                this->backgroundTasks.erase(key);

                return *(storedAsset.get());
            }
        }

        // lets not use shared ptrs for keys? causes paradigm issues when using the keys in ECS components
        AsyncCacheTask<V*> getAsync(std::shared_ptr<K> keyObj) {
            ZoneScoped;
            size_t key = keyObj->hashCode();

            // quick check with shared read lock
            {
                std::shared_lock<std::shared_mutex> lock(this->lock);

                // already loaded
                auto cacheIt = cache.find(key);
                if (cacheIt != cache.end())
                    return AsyncCacheTask<V*>(cacheIt->second.get());

                // currently loading
                auto taskIt = backgroundTasks.find(key);
                if (taskIt != backgroundTasks.end())
                    return AsyncCacheTask<V*>(taskIt->second);
            }

            // write
            {
                std::lock_guard<std::shared_mutex> lock(this->lock);

                // already loaded
                auto cacheIt = cache.find(key);
                if (cacheIt != cache.end())
                    return AsyncCacheTask<V*>(cacheIt->second.get());

                // double check
                auto taskIt = backgroundTasks.find(key);
                if (taskIt != backgroundTasks.end())
                    return AsyncCacheTask<V*>(taskIt->second);

                auto task = backgroundTasks[key] = workerPool.submitShared([this, key, keyObj]() {
                    // need to handle exception to propagate to caller
                    auto asset = this->create(keyObj);

                    {
                        std::lock_guard<std::shared_mutex> lock(this->lock);
                        auto& storedAsset = this->cache[key] = std::move(asset);
                        this->backgroundTasks.erase(key);

                        return storedAsset.get();
                    }
                });

                return AsyncCacheTask<V*>(task);
            }
        }
    };
} // namespace ke