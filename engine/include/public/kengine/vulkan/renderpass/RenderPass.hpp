#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/GpuImage.hpp>
#include <glm/vec2.hpp>
#include <type_traits>
#include <memory>
#include <vector>

class RenderPass;

class RenderTarget {
public:
    RenderTarget(VkDevice vkDevice)
        : vkDevice(vkDevice) { }

    ~RenderTarget();

    VkFramebuffer getVkFramebuffer() const {
        return vkFrameBuffer;
    }

    virtual VkFramebuffer createFramebuffer(
        RenderPass& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    ) = 0;

    virtual void init(
        RenderPass& renderPass,
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    );

protected:
    VkDevice vkDevice;

private:
    VkFramebuffer vkFrameBuffer = VK_NULL_HANDLE;

};

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

    const VkRenderPass getVkRenderPass() const {
        return vkRenderPass;
    }

    virtual void init();
    virtual void createRenderTargets(
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView>& sharedImageViews,
        const glm::uvec2& extents
    ) = 0;

    virtual void begin(RenderPassContext& cxt) = 0;
    virtual void end(RenderPassContext& cxt) = 0;

    const GpuImageView& getDepthStencilImageView() const;

private:
    const ColorFormatAndSpace& colorFormatAndSpace;

    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    std::vector<std::unique_ptr<RenderTarget>> renderTargets;
    std::unique_ptr<GpuImageView> depthStencilImageView;

protected:
    const VkDevice vkDevice;

    const VkDevice getVkDevice() const {
        return vkDevice;
    }

    void addRenderTarget(std::unique_ptr<RenderTarget>&& rt) {
        renderTargets.push_back(std::move(rt));
    }

    const ColorFormatAndSpace& getColorFormatAndSpace() const {
        return colorFormatAndSpace;
    }

    const GpuImageView* getDepthStencil() const {
        return depthStencilImageView.get();
    }

    const void setDepthStencil(std::unique_ptr<GpuImageView> ds) {
        depthStencilImageView = std::move(ds);
    }

    void freeRenderTargets();

    const RenderTarget* getRenderTarget(size_t renderTargetIndex) const;

    virtual VkRenderPass createVkRenderPass() = 0;
    virtual std::unique_ptr<GpuImageView> createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2 extents) {
        throw std::runtime_error("createDepthStencil is not implemented.");
    }

    virtual void createRenderTargets(
        VmaAllocator vmaAllocator,
        const std::vector<VkImageView> sharedImageViews,
        const glm::uvec2 extents
    ) = 0;
};