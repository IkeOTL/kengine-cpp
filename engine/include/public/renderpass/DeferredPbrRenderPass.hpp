#pragma once
#include <renderpass/RenderPass.hpp>

class DeferredPbrRenderPass : public RenderPass {
public:
    DeferredPbrRenderPass(VkDevice vkDevice, ColorFormatAndSpace& colorFormatAndSpace)
        : RenderPass(vkDevice, colorFormatAndSpace) { }

    // Inherited via RenderPass
    virtual void createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::ivec2& extents) override;
    virtual void begin(RenderPassContext& cxt) override;
    virtual void end(RenderPassContext& cxt) override;


    DeferredPbrRenderPass(const DeferredPbrRenderPass&) = delete;
    DeferredPbrRenderPass& operator=(const DeferredPbrRenderPass&) = delete;

    DeferredPbrRenderPass(DeferredPbrRenderPass&&) = default;
    DeferredPbrRenderPass& operator=(DeferredPbrRenderPass&&) = default;

protected:
    virtual std::unique_ptr<VkRenderPass> createVkRenderPass() override;
    virtual std::unique_ptr<VmaImage::ImageAndView> createDepthStencil(VmaAllocator vmaAllocator, glm::ivec2 extents) override;
    virtual std::unique_ptr<RenderTarget> createRenderTarget(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::ivec2& extents, const int renderTargetIndex) override;
};