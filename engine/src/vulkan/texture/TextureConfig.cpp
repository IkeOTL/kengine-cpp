#include <kengine/vulkan/texture/TextureConfig.hpp>

bool TextureConfig::operator==(const TextureConfig& other) const {
    if (this == &other)
        return true;

    return format == other.format &&
        imageType == other.imageType &&
        imageViewType == other.imageViewType &&
        channels == other.channels &&
        dstStageMask == other.dstStageMask &&
        dstAccessMask == other.dstAccessMask &&
        mipmaps == other.mipmaps &&
        textureKey == other.textureKey;
}

bool TextureConfig::operator!=(const TextureConfig& other) const {
    return !(*this == other);
}

// todo: review this hashing
namespace std {
    template<>
    struct hash<TextureConfig> {
        size_t operator()(const TextureConfig& p) const noexcept {
            size_t hash = 7;
            hash = 67 * hash + std::hash<std::string>()(p.textureKey);
            hash = 67 * hash + p.format;
            hash = 67 * hash + p.imageType;
            hash = 67 * hash + p.imageViewType;
            hash = 67 * hash + p.channels;
            hash = 67 * hash + (int)(p.dstStageMask ^ (p.dstStageMask >> 32));
            hash = 67 * hash + (int)(p.dstAccessMask ^ (p.dstAccessMask >> 32));
            hash = 67 * hash + (p.mipmaps ? 1 : 0);
            return hash;
        }
    };
}