#pragma once
#include <VulkanInclude.hpp>
#include <type_traits>
#include <memory>
#include <vector>
#include <ColorFormatAndSpace.hpp>
#include <glm/vec2.hpp>
#include <VmaImage.hpp>
#include <renderpass/RenderTarget.hpp>

class RenderPass {

public:
    struct RenderPassContext {
        const int renderPassIndex;
        const int renderTargetIndex;
        const VkCommandBuffer cmd;
        const glm::uvec2 extents;
    };

    RenderPass(VkDevice vkDevice, ColorFormatAndSpace& colorFormatAndSpace)
        : vkDevice(vkDevice), colorFormatAndSpace(colorFormatAndSpace) { }

    virtual ~RenderPass() = default;

    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(RenderPass&&) = default;

    virtual void init();
    virtual void createRenderTargets(
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    ) = 0;

    virtual void begin(RenderPassContext& cxt) = 0;
    virtual void end(RenderPassContext& cxt) = 0;

private:
    const VkDevice vkDevice;
    const ColorFormatAndSpace& colorFormatAndSpace;

    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    std::vector<std::unique_ptr<RenderTarget>> renderTargets;
    std::unique_ptr<VmaImage::ImageAndView> depthStencilImageView = nullptr;

protected:
    const VkDevice getVkDevice() const {
        return vkDevice;
    }

    const ColorFormatAndSpace& getColorFormatAndSpace() const {
        return colorFormatAndSpace;
    }

    const VkRenderPass getVkRenderPass() const {
        return vkRenderPass;
    }

    const VmaImage::ImageAndView* getDepthStencil() const {
        return depthStencilImageView ? depthStencilImageView.get() : nullptr;
    }

    void freeRenderTargets();

    const RenderTarget* getRenderTarget(size_t renderTargetIndex) const;

    virtual VkRenderPass createVkRenderPass() = 0;
    virtual std::unique_ptr<VmaImage::ImageAndView> createDepthStencil(VmaAllocator vmaAllocator, glm::uvec2& extents) = 0;

    virtual std::unique_ptr<RenderTarget> createRenderTarget(
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents,
        const int renderTargetIndex
    ) = 0;
};