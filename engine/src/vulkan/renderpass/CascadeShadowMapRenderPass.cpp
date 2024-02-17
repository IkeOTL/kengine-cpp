#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>
#include <kengine/vulkan/ShadowCascade.hpp>

// render target
VkFramebuffer CascadeShadowMapRenderTarget::createFramebuffer(RenderPass& renderPass, VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) {
    lol

        return nullptr;
}

void CascadeShadowMapRenderPass::init(VulkanContext& vkCtx) {
    RenderPass::init(vkCtx);

    createRenderTargets(vkCtx.getVmaAllocator(), {}, glm::uvec2(4096));
}

void CascadeShadowMapRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) {
    setDepthStencil(createDepthStencil(vmaAllocator, extents));
    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto fb = std::make_unique<CascadeShadowMapRenderTarget>(vkDevice, *getDepthStencil(), i);
        fb->init(*this, vmaAllocator, {}, extents);
        addRenderTarget(std::move(fb));
    }
}

// render pass
void CascadeShadowMapRenderPass::begin(RenderPassContext& cxt) {
    lol
}

void CascadeShadowMapRenderPass::end(RenderPassContext& cxt) {
    lol
}

VkRenderPass CascadeShadowMapRenderPass::createVkRenderPass() {
    lol

        return nullptr;
}

std::unique_ptr<GpuImageView> CascadeShadowMapRenderPass::createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2 extents) {
    lol
        return nullptr;
}