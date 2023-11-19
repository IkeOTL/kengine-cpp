#pragma once
#include "VmaInclude.hpp"
#include <memory>

class VmaImage {

public:
    struct ImageAndView {
        const std::shared_ptr<VmaImage> vmaImage;
        const VkImageView imageView;
    };

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

private:
    const VmaAllocator vmaAllocator;
    const VkImage vkImage;
    const VmaAllocation vmaAllocation;

};