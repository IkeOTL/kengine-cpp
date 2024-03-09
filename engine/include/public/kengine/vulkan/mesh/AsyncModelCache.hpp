#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/mesh/ModelConfig>

class Model;

class AsyncModelCache : AsyncAssetCache<Model, ModelConfig>{

public:
    AsyncModelCache(ExecutorService& workerPool)
        : AsyncAssetCache(workerPool) {}

    std::unique_ptr<Model> create(std::shared_ptr<ModelConfig> keyObj) override;
};