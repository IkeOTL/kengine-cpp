#pragma once
#include "VmaInclude.hpp"
#include <memory>

struct GpuImage {
    const VkDevice vkDevice;
    const VmaAllocator vmaAllocator;
    const VkImage vkImage;
    const VmaAllocation vmaAllocation;

    ~GpuImage() {
        vmaDestroyImage(vmaAllocator, vkImage, vmaAllocation);
    }
};

struct GpuImageView {
    const std::shared_ptr<GpuImage> gpuImage;
    const VkImageView imageView;

    ~GpuImageView() {
        vkDestroyImageView(gpuImage.get()->vkDevice, imageView, nullptr);
    }
};