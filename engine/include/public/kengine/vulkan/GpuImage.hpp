#pragma once
#include "VmaInclude.hpp"
#include <memory>

namespace ke {
    class GpuImage {
    public:
        const VkDevice vkDevice;
        const VmaAllocator vmaAllocator;
        const VkImage vkImage;
        const VmaAllocation vmaAllocation;
        const VkImageCreateInfo imageInfo;

        GpuImage(VkDevice vkDevice, VmaAllocator vmaAllocator, VkImage vkImage, VmaAllocation vmaAllocation, VkImageCreateInfo imageInfo)
            : vkDevice(vkDevice),
              vmaAllocator(vmaAllocator),
              vkImage(vkImage),
              vmaAllocation(vmaAllocation),
              imageInfo(imageInfo) {}

        ~GpuImage() {
            vmaDestroyImage(vmaAllocator, vkImage, vmaAllocation);
        }
    };

    class GpuImageView {
    public:
        const std::shared_ptr<GpuImage> gpuImage;
        const VkImageView imageView;
        const VkImageViewCreateInfo imageViewInfo;

        GpuImageView(std::shared_ptr<GpuImage> gpuImage, VkImageView imageView, VkImageViewCreateInfo imageViewInfo)
            : gpuImage(gpuImage),
              imageView(imageView),
              imageViewInfo(imageViewInfo) {}

        ~GpuImageView() {
            vkDestroyImageView(gpuImage.get()->vkDevice, imageView, nullptr);
        }
    };
} // namespace ke