#pragma once
#include "VmaInclude.hpp"
#include <memory>

struct GpuImage {
    const VkDevice vkDevice;
    const VmaAllocator vmaAllocator;
    const VkImage vkImage;
    const VmaAllocation vmaAllocation;
    const VkImageCreateInfo imageInfo;

    ~GpuImage() {
        vmaDestroyImage(vmaAllocator, vkImage, vmaAllocation);
    }
};

struct GpuImageView {
    const std::shared_ptr<GpuImage> gpuImage;
    const VkImageView imageView;
    const VkImageViewCreateInfo imageViewInfo;

    ~GpuImageView() {
        vkDestroyImageView(gpuImage.get()->vkDevice, imageView, nullptr);
    }
};