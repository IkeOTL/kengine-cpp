#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

std::future<Texture2d*> AsyncTextureCache::get(std::string key) {
    return AsyncAssetCache::get(TextureConfig(key));
}

std::unique_ptr<Texture2d> AsyncTextureCache::create(TextureConfig key) {
    return factory.loadTexture(key);
}

