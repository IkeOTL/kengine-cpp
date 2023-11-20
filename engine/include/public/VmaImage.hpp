#pragma once
#include "VmaInclude.hpp"
#include <memory>
 rename GpuImage
class VmaImage {

public:
    create GpuImageView
    struct ImageAndView {
        const std::shared_ptr<VmaImage> vmaImage;
        const VkImageView imageView;

        ~ImageAndView() {
            vkDestroyImageView(vmaImage.get()->vkDevice, imageView, nullptr);
        }
    };

    VmaImage(
        VkDevice vkDevice,
        VmaAllocator vmaAllocator,
        VkImage vkImage,
        VmaAllocation vmaAllocation
    )
        : vkDevice(vkDevice),
        vmaAllocator(vmaAllocator),
        vkImage(vkImage),
        vmaAllocation(vmaAllocation)
    { }

    ~VmaImage() {
        vmaDestroyImage(vmaAllocator, vkImage, vmaAllocation);
    }

    VkImage getVkImage() {
        return vkImage;
    }

private:
    const VkDevice vkDevice;
    const VmaAllocator vmaAllocator;
    const VkImage vkImage;
    const VmaAllocation vmaAllocation;

};