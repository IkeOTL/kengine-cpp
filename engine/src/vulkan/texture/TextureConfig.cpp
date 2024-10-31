#include <kengine/vulkan/texture/TextureConfig.hpp>

namespace ke {
    bool TextureConfig::operator==(const TextureConfig& other) const {
        if (this == &other)
            return true;

        return format == other.format && imageType == other.imageType && imageViewType == other.imageViewType && channels == other.channels && dstStageMask == other.dstStageMask && dstAccessMask == other.dstAccessMask && mipmaps == other.mipmaps && textureKey == other.textureKey;
    }

    bool TextureConfig::operator!=(const TextureConfig& other) const {
        return !(*this == other);
    }

    size_t TextureConfig::hashCode() const noexcept {
        size_t hash = 7;
        hash = 67 * hash + std::hash<std::string>()(textureKey);
        hash = 67 * hash + static_cast<int>(format);
        hash = 67 * hash + static_cast<int>(imageType);
        hash = 67 * hash + static_cast<int>(imageViewType);
        hash = 67 * hash + channels;
        hash = 67 * hash + (int)(dstStageMask ^ (dstStageMask >> 32));
        hash = 67 * hash + (int)(dstAccessMask ^ (dstAccessMask >> 32));
        hash = 67 * hash + (mipmaps ? 1 : 0);
        return hash;
    }
} // namespace ke
