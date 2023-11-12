#include "renderpass/RenderTarget.hpp"
#include "renderpass/RenderPass.hpp"

void RenderTarget::init(
    RenderPass& renderPass,
    VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews,
    const glm::ivec2& extents
) {
    vkFrameBuffer = createFramebuffer(renderPass, vmaAllocator, sharedImageViews, extents);
}

RenderTarget::~RenderTarget() {
    if (vkFrameBuffer)
        vkDestroyFramebuffer(vkDevice, vkFrameBuffer, VK_NULL_HANDLE);
}