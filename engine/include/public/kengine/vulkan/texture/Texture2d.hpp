#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuImage.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <functional>
#include <string>

class Texture2d {
private:
    std::shared_ptr<GpuImage> gpuImage;
    std::unique_ptr<GpuImageView> gpuImageView;
    uint32_t mipLevels;
    uint32_t width, height;

    void init(VulkanContext& vkCxt, char* image, uint32_t width, uint32_t height,
        VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels,
        VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, bool generateMipMaps);

public:
    Texture2d(VulkanContext& vkCxt, char* image, uint32_t width, uint32_t height,
        VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels,
        VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccesMask, bool generateMipMaps) {
        init(vkCxt, image, width, height, format, imageType, imageViewType, channels,
            dstStageMask, dstAccesMask, generateMipMaps);
    }
};