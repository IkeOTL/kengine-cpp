#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>

// render target
VkFramebuffer CascadeShadowMapRenderTarget::createFramebuffer(RenderPass& renderPass, VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) {
    lol

    return nullptr;
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

void CascadeShadowMapRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) {
    lol
}