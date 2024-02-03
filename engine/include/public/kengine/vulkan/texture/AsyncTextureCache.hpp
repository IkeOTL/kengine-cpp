#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/texture/TextureConfig.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>
#include <kengine/vulkan/texture/TextureFactory.hpp>

class AsyncTextureCache : AsyncAssetCache<Texture2d, TextureConfig> {
private:
    TextureFactory& factory;

public:
    AsyncTextureCache(TextureFactory& factory, ExecutorService& workerPool)
        : AsyncAssetCache(workerPool), factory(factory) {}

    std::future<Texture2d*> get(std::string key);

    std::unique_ptr<Texture2d> create(TextureConfig key) override;
};