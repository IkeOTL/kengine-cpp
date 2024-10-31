#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/Hashable.hpp>
#include <functional>
#include <string>

namespace ke {
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
            int channels, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, bool mipmaps)
            : textureKey(textureKey),
              format(format),
              imageType(imageType),
              imageViewType(imageViewType),
              channels(channels),
              dstStageMask(dstStageMask),
              dstAccessMask(dstAccessMask),
              mipmaps(mipmaps) {}

        TextureConfig(std::string textureKey)
            : TextureConfig(textureKey, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, 4, true) {}

        TextureConfig(std::string textureKey, VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels, bool mipmaps)
            : TextureConfig(textureKey,
                  format,
                  imageType,
                  imageViewType,
                  channels,
                  VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                  VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                  mipmaps) {}

        bool operator==(const TextureConfig& other) const;
        bool operator!=(const TextureConfig& other) const;

        friend struct std::hash<TextureConfig>;

        const std::string& getTextureKey() const {
            return textureKey;
        }

        VkFormat getFormat() const {
            return format;
        }

        VkImageType getImageType() const {
            return imageType;
        }

        VkImageViewType getImageViewType() const {
            return imageViewType;
        }

        bool hasMipmaps() const {
            return mipmaps;
        }

        int getChannels() const {
            return channels;
        }

        VkAccessFlags2 getDstStageMask() const {
            return dstStageMask;
        }

        VkAccessFlags2 getDstAccessMask() const {
            return dstAccessMask;
        }

        size_t hashCode() const noexcept override;
    };
} // namespace ke

namespace std {
    template <>
    struct hash<ke::TextureConfig> {
        size_t operator()(const ke::TextureConfig& p) const noexcept {
            return p.hashCode();
        }
    };
} // namespace std