#pragma once
#include <kengine/vulkan/renderpass/RenderPass.hpp>

class DeferredPbrRenderPass : public RenderPass {
public:
    DeferredPbrRenderPass(VkDevice vkDevice, ColorFormatAndSpace& colorFormatAndSpace)
        : RenderPass(vkDevice, colorFormatAndSpace) { }

    // Inherited via RenderPass
    void createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) override;
    void begin(RenderPassContext& cxt) override;
    void end(RenderPassContext& cxt) override;


    DeferredPbrRenderPass(const DeferredPbrRenderPass&) = delete;
    DeferredPbrRenderPass& operator=(const DeferredPbrRenderPass&) = delete;

    DeferredPbrRenderPass(DeferredPbrRenderPass&&) = default;
    DeferredPbrRenderPass& operator=(DeferredPbrRenderPass&&) = default;

protected:
    VkRenderPass createVkRenderPass() override;
    std::unique_ptr<GpuImageView> createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2& extents) override;
    std::unique_ptr<RenderTarget> createRenderTarget(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents, const int renderTargetIndex) override;

};