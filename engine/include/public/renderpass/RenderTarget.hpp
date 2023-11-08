#pragma once
#include <VulkanContext.hpp>

#include <vulkan/vulkan.h>
#include "RenderPass.hpp"
#include <glm/vec2.hpp>

class RenderTarget {

public:
    RenderTarget(VkDevice vkDevice);

    VkFramebuffer createFramebuffer(VmaAllocator vmaAllocator, std::vector<VkImageView> sharedImageViews, glm::ivec2 extents);

    void init(VmaAllocator vmaAllocator, std::vector<VkImageView> sharedImageViews, glm::ivec2 extents) {
        vkFrameBuffer = createFramebuffer(vmaAllocator, sharedImageViews, extents);
    }

    void vkDispose() {
        if (vkFrameBuffer)
            vkDestroyFramebuffer(vkDevice, vkFrameBuffer, VK_NULL_HANDLE);
    }

private:
    RenderPass<RenderTarget>& renderPass;
    VkDevice vkDevice;
    VkFramebuffer vkFrameBuffer = VK_NULL_HANDLE;

};