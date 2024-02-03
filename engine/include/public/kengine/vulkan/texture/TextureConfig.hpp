#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <functional>
#include <string>

class TextureConfig {
private:
    std::string textureKey;
    int format, imageType, imageViewType;
    int channels;
    VkFlags64 dstStageMask, dstAccessMask;
    bool mipmaps;

public:
    TextureConfig(std::string textureKey, int format, int imageType, int imageViewType, int channels, bool mipmaps)
        : textureKey(textureKey),
        format(format),
        imageType(imageType),
        imageViewType(imageViewType),
        channels(channels),
        dstStageMask(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR),
        dstAccessMask(VK_ACCESS_2_SHADER_SAMPLED_READ_BIT),
        mipmaps(mipmaps)
    {}

    bool operator==(const TextureConfig& other) const;
    bool operator!=(const TextureConfig& other) const;

    friend struct std::hash<TextureConfig>;

    std::string getTextureKey() {
        return textureKey;
    }

    int getFormat() {
        return format;
    }

    int getImageType() {
        return imageType;
    }

    int getImageViewType() {
        return imageViewType;
    }

    bool hasMipmaps() {
        return mipmaps;
    }

    int getChannels() {
        return channels;
    }

    VkFlags64 getDstStageMask() {
        return dstStageMask;
    }

    VkFlags64 getDstAccessMask() {
        return dstAccessMask;
    }
};

namespace std {
    template<> struct hash<TextureConfig>;
}