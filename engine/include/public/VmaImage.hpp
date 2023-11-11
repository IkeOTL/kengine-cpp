#pragma once
#include "VmaInclude.hpp"

class VmaImage {

private:
    const VmaAllocator vmaAllocator;
    const VkImage vkImage;
    const VmaAllocation vmaAllocation;

public:
    VmaImage(
        VmaAllocator vmaAllocator,
        VkImage vkImage,
        VmaAllocation vmaAllocation
    )
        : vmaAllocator(vmaAllocator),
        vkImage(vkImage),
        vmaAllocation(vmaAllocation)
    { }

    ~VmaImage() {
        vmaDestroyImage(vmaAllocator, vkImage, vmaAllocation);
    }

    VkImage getVkImage() {
        return vkImage;
    }

};