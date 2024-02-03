#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuImage.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <functional>
#include <string>

class Texture2d {
private:
    std::unique_ptr<GpuImage> gpuImage;
    std::unique_ptr<GpuImageView> imageView;
    uint32_t mipLevels;
    int width, height;

    void init(VulkanContext& vkCxt, char* image, int width, int height,
        VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels,
        VkFlags64 dstStageMask, VkFlags64 dstAccessMask, bool generateMipMaps);

public:
    Texture2d(VulkanContext& vkCxt, char* image, int width, int height,
        VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels,
        VkFlags64 dstStageMask, VkFlags64 dstAccesMask, bool generateMipMaps) {
        init(vkCxt, image, width, height, format, imageType, imageViewType, channels,
            dstStageMask, dstAccesMask, generateMipMaps);
    }
};