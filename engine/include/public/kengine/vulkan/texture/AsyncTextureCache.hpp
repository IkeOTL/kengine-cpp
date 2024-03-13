#pragma once
#include <kengine/vulkan/texture/TextureConfig.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>
#include <kengine/vulkan/texture/TextureFactory.hpp>
#include <kengine/vulkan/AsyncAssetCache.hpp>

class AsyncTextureCache : public AsyncAssetCache<Texture2d, TextureConfig> {
private:
    TextureFactory& factory;

protected:
    std::unique_ptr<Texture2d> create(std::shared_ptr<TextureConfig> keyObj) override;

public:
    AsyncTextureCache(TextureFactory& factory, ExecutorService& workerPool)
        : AsyncAssetCache(workerPool), factory(factory) {}
};