#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

namespace ke {
    std::unique_ptr<Texture2d> AsyncTextureCache::create(std::shared_ptr<TextureConfig> keyObj) {
        return factory.loadTexture(*keyObj);
    }
} // namespace ke