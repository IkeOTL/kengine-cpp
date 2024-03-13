#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

std::unique_ptr<Texture2d> AsyncTextureCache::create(std::shared_ptr<TextureConfig> keyObj) {
    return factory.loadTexture(*keyObj);
}

