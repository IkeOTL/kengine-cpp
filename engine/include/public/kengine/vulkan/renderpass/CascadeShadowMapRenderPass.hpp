#pragma once
#include <kengine/vulkan/renderpass/RenderPass.hpp>

namespace ke {
    class CascadeShadowMapRenderTarget : public RenderTarget {
    private:
        const GpuImageView& shadowMapDepthImage;
        const uint32_t cascadeIndex;
        std::unique_ptr<GpuImageView> cascadeImageView = nullptr;

        VkFramebuffer createFramebuffer(
            RenderPass& renderPass,
            VmaAllocator vmaAllocator,
            const std::vector<VkImageView>& sharedImageViews,
            const glm::uvec2& extents) override;

    public:
        CascadeShadowMapRenderTarget(VkDevice vkDevice, const GpuImageView& shadowMapDepthImage, uint32_t cascadeIndex)
            : RenderTarget(vkDevice),
              shadowMapDepthImage(shadowMapDepthImage),
              cascadeIndex(cascadeIndex) {}

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
            : RenderPass(vkDevice, colorFormatAndSpace) {}

        static inline std::unique_ptr<RenderPass> create(VkDevice vkDevice, ColorFormatAndSpace& colorFormatAndSpace) {
            return std::make_unique<CascadeShadowMapRenderPass>(vkDevice, colorFormatAndSpace);
        }

        void begin(RenderPassContext& cxt) override;
        void end(RenderPassContext& cxt) override;

        CascadeShadowMapRenderPass(const CascadeShadowMapRenderPass&) = delete;
        CascadeShadowMapRenderPass& operator=(const CascadeShadowMapRenderPass&) = delete;

        CascadeShadowMapRenderPass(CascadeShadowMapRenderPass&&) = default;
        CascadeShadowMapRenderPass& operator=(CascadeShadowMapRenderPass&&) = default;

        void init(VulkanContext& vkCtx) override;

    protected:
        VkRenderPass createVkRenderPass() override;
        std::unique_ptr<GpuImageView> createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2 extents) override;
        void createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) override;
    };
} // namespace ke