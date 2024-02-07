#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

std::shared_future<Texture2d*> AsyncTextureCache::get(std::string key) {
    auto ptr = std::make_shared<TextureConfig>(key);
    return AsyncAssetCache::get(ptr);
}

std::unique_ptr<Texture2d> AsyncTextureCache::create(std::shared_ptr<TextureConfig> keyObj) {
    return factory.loadTexture(*keyObj);
}

