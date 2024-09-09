#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/mesh/ModelConfig.hpp>
#include <kengine/vulkan/mesh/ModelFactory.hpp>

class Model;

class AsyncModelCache : public AsyncAssetCache<Model, ModelConfig> {
private:
    ModelFactory& factory;

protected:
    std::unique_ptr<Model> create(std::shared_ptr<ModelConfig> keyObj) override;

public:
    AsyncModelCache(ModelFactory& factory, ExecutorService& workerPool)
        : AsyncAssetCache(workerPool), factory(factory) {}

};