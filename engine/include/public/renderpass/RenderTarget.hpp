#pragma once
#include <VulkanContext.hpp>

#include <vulkan/vulkan.h>
#include "RenderPass.hpp"
#include <glm/vec2.hpp>

class RenderTarget {

public:
    RenderTarget(VkDevice vkDevice)
        : vkDevice(vkDevice) { }

    virtual VkFramebuffer createFramebuffer(
        RenderPass<RenderTarget>& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::ivec2& extents
    ) = 0;

    void init(
        RenderPass<RenderTarget>& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::ivec2& extents
    ) {
        vkFrameBuffer = createFramebuffer(renderPass, vmaAllocator, sharedImageViews, extents);
    }

    void vkDispose() {
        if (vkFrameBuffer)
            vkDestroyFramebuffer(vkDevice, vkFrameBuffer, VK_NULL_HANDLE);
    }

private:
    VkDevice vkDevice;
    VkFramebuffer vkFrameBuffer = VK_NULL_HANDLE;

};