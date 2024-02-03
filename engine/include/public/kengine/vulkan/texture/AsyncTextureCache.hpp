#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/texture/TextureConfig.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>

class AsyncTextureCache : AsyncAssetCache<Texture2d, TextureConfig> {
private:


public:

    AsyncTextureCache(ExecutorService& workerPool)
        : AsyncAssetCache(workerPool) {}
};