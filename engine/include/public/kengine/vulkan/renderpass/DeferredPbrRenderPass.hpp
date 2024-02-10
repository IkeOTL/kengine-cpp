#pragma once
#include <kengine/vulkan/renderpass/RenderPass.hpp>

class DeferredPbrRenderTarget : public RenderTarget {
private:
    std::unique_ptr<GpuImageView> albedoImage;
    std::unique_ptr<GpuImageView> positionImage;
    std::unique_ptr<GpuImageView> normalImage;
    std::unique_ptr<GpuImageView> ormImage;
    std::unique_ptr<GpuImageView> emissiveImage;

    DeferredPbrRenderTarget(VkDevice vkDevice)
        : RenderTarget(vkDevice) {}

    VkFramebuffer createFramebuffer(
        RenderPass& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    ) override;

    std::unique_ptr<GpuImageView>&& createAttachmentImage(VmaAllocator vmaAllocator, VkFormat format, VkImageUsageFlags imageUsage,
        VmaMemoryUsage memUsage, VkImageAspectFlags viewAspectMask, const glm::uvec2 extents);

public:
    const GpuImageView& getAlbedoImage() const;
    const GpuImageView& getPositionImage() const;
    const GpuImageView& getNormalImage() const;
    const GpuImageView& getOrmImage() const;
    const GpuImageView& getEmissiveImage() const;
};

class DeferredPbrRenderPass : public RenderPass {
public:
    DeferredPbrRenderPass(VkDevice vkDevice, ColorFormatAndSpace& colorFormatAndSpace)
        : RenderPass(vkDevice, colorFormatAndSpace) { }

    void begin(RenderPassContext& cxt) override;
    void end(RenderPassContext& cxt) override;

    DeferredPbrRenderPass(const DeferredPbrRenderPass&) = delete;
    DeferredPbrRenderPass& operator=(const DeferredPbrRenderPass&) = delete;

    DeferredPbrRenderPass(DeferredPbrRenderPass&&) = default;
    DeferredPbrRenderPass& operator=(DeferredPbrRenderPass&&) = default;

protected:
    VkRenderPass createVkRenderPass() override;
    std::unique_ptr<GpuImageView> createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2 extents) override;
    void createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) override;
};