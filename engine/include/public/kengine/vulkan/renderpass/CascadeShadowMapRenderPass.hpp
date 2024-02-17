#pragma once
#include <kengine/vulkan/renderpass/RenderPass.hpp>

class CascadeShadowMapRenderTarget : public RenderTarget {
private:
    GpuImageView& shadowMapDepthImage;
    const uint32_t cascadeIndex;
    std::unique_ptr<GpuImageView> cascadeImageView = nullptr;

    VkFramebuffer createFramebuffer(
        RenderPass& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    ) override;

public:
    CascadeShadowMapRenderTarget(VkDevice vkDevice, GpuImageView& shadowMapDepthImage, uint32_t cascadeIndex)
        : RenderTarget(vkDevice), shadowMapDepthImage(shadowMapDepthImage), cascadeIndex(cascadeIndex) {}

    const GpuImageView& getCascadeImageView() const {
        return *cascadeImageView;
    }

    const GpuImageView& getShadowMapDepthImage() const {
        return shadowMapDepthImage;
    }
};

class CascadeShadowMapRenderPass : public RenderPass {
public:
    CascadeShadowMapRenderPass(VkDevice vkDevice, ColorFormatAndSpace& colorFormatAndSpace)
        : RenderPass(vkDevice, colorFormatAndSpace) { }

    void begin(RenderPassContext& cxt) override;
    void end(RenderPassContext& cxt) override;

    CascadeShadowMapRenderPass(const CascadeShadowMapRenderPass&) = delete;
    CascadeShadowMapRenderPass& operator=(const CascadeShadowMapRenderPass&) = delete;

    CascadeShadowMapRenderPass(CascadeShadowMapRenderPass&&) = default;
    CascadeShadowMapRenderPass& operator=(CascadeShadowMapRenderPass&&) = default;

protected:
    VkRenderPass createVkRenderPass() override;
    std::unique_ptr<GpuImageView> createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2 extents) override;
    void createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) override;
};