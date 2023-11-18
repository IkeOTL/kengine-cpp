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
        const int frameBufferIndex;
        const VkCommandBuffer cmd;
        const glm::ivec2 extents;
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
        const glm::ivec2& extents
    ) = 0;

    virtual void begin(RenderPassContext& cxt) = 0;
    virtual void end(RenderPassContext& cxt) = 0;

private:
    const VkDevice vkDevice;
    const ColorFormatAndSpace& colorFormatAndSpace;

    std::unique_ptr<VkRenderPass> vkRenderPass; // add deleter
    std::vector<std::unique_ptr<RenderTarget>> renderTargets;
    std::unique_ptr<VmaImage::ImageAndView> depthStencilImageView;

protected:
    VkDevice getVkDevice() {
        return vkDevice;
    }

    const ColorFormatAndSpace& getColorFormatAndSpace() {
        return colorFormatAndSpace;
    }

    VkRenderPass* getVkRenderPass() {
        return vkRenderPass.get();
    }

    virtual std::unique_ptr<VkRenderPass> createVkRenderPass() = 0;
    virtual std::unique_ptr<VmaImage::ImageAndView> createDepthStencil(VmaAllocator vmaAllocator, glm::ivec2 extents) = 0;

    virtual std::unique_ptr<RenderTarget> createRenderTarget(
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::ivec2& extents,
        const int renderTargetIndex
    ) = 0;
};