#pragma once
#include <VulkanInclude.hpp>
#include <VmaInclude.hpp>
#include <vector>
#include <glm/vec2.hpp>

class RenderPass;

class RenderTarget {

public:
    RenderTarget(VkDevice vkDevice)
        : vkDevice(vkDevice) { }

    ~RenderTarget();

    VkFramebuffer getVkFramebuffer() const {
        return vkFrameBuffer;
    }

    virtual VkFramebuffer createFramebuffer(
        RenderPass& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    ) = 0;

    void init(
        RenderPass& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    );

private:
    VkDevice vkDevice;
    VkFramebuffer vkFrameBuffer = VK_NULL_HANDLE;

};