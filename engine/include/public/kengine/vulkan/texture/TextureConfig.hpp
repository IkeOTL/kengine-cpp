#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/Hashable.hpp>
#include <functional>
#include <string>

class TextureConfig : Hashable {
private:
    std::string textureKey;
    VkFormat format;
    VkImageType imageType;
    VkImageViewType imageViewType;
    int channels;
    VkAccessFlags2 dstStageMask, dstAccessMask;
    bool mipmaps;

public:
    TextureConfig(std::string textureKey, VkFormat format,
        VkImageType imageType, VkImageViewType imageViewType,
        int channels, VkAccessFlags2 dstStageMask, VkAccessFlags2
        dstAccessMask, bool mipmaps)
        : textureKey(textureKey),
        format(format),
        imageType(imageType),
        imageViewType(imageViewType),
        channels(channels),
        dstStageMask(dstStageMask),
        dstAccessMask(dstAccessMask),
        mipmaps(mipmaps)
    {}

    TextureConfig(std::string textureKey)
        : TextureConfig(textureKey, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, 4, true)
    {}

    TextureConfig(std::string textureKey, VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels, bool mipmaps)
        : TextureConfig(textureKey,
            format,
            imageType,
            imageViewType,
            channels,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            mipmaps)
    {}

    bool operator==(const TextureConfig& other) const;
    bool operator!=(const TextureConfig& other) const;

    friend struct std::hash<TextureConfig>;

    std::string getTextureKey() {
        return textureKey;
    }

    VkFormat getFormat() {
        return format;
    }

    VkImageType getImageType() {
        return imageType;
    }

    VkImageViewType getImageViewType() {
        return imageViewType;
    }

    bool hasMipmaps() {
        return mipmaps;
    }

    int getChannels() {
        return channels;
    }

    VkAccessFlags2 getDstStageMask() {
        return dstStageMask;
    }

    VkAccessFlags2 getDstAccessMask() {
        return dstAccessMask;
    }

    size_t hashCode() const noexcept override;
};

namespace std {
    template<>
    struct hash<TextureConfig> {
        size_t operator()(const TextureConfig& p) const noexcept {
            return p.hashCode();
        }
    };
}