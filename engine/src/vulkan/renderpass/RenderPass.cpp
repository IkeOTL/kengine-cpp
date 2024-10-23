#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>

RenderPass::~RenderPass() {
    vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
}

// render target
void RenderTarget::init(
    RenderPass& renderPass,
    VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews,
    const glm::uvec2& extents
) {
    vkFrameBuffer = createFramebuffer(renderPass, vmaAllocator, sharedImageViews, extents);
}

RenderTarget::~RenderTarget() {
    if (vkFrameBuffer)
        vkDestroyFramebuffer(vkDevice, vkFrameBuffer, VK_NULL_HANDLE);
}

// render pass
void RenderPass::init(VulkanContext& vkCtx) {
    vkRenderPass = createVkRenderPass();
}

void RenderPass::freeRenderTargets() {
    depthStencilImageView = nullptr;
    renderTargets.clear();
}

const GpuImageView& RenderPass::getDepthStencilImageView() const {
    if (!depthStencilImageView)
        throw std::runtime_error("Missing depthStencil imageview.");

    return *depthStencilImageView;
}