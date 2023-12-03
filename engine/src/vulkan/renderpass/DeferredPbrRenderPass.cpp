#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>

void DeferredPbrRenderPass::begin(RenderPassContext& cxt) {
    const auto clearCount = 7;
    VkClearValue clearVal[clearCount]{};
    clearVal[0].color = { {0, 0, 0, 0} };
    clearVal[1].color = { {0, 0, 0, 0} };
    clearVal[2].color = { {0, 0, 0, 0} };
    clearVal[3].color = { {0, 0, 0, 0} };
    clearVal[4].color = { {0, 0, 0, 0} };
    clearVal[5].color = { {0, 0, 0, 0} };
    clearVal[6].depthStencil = { 1, 0 };

    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = getVkRenderPass();

    rpInfo.clearValueCount = clearCount;
    rpInfo.pClearValues = clearVal;

    rpInfo.framebuffer = getRenderTarget(cxt.renderTargetIndex)->getVkFramebuffer();

    rpInfo.renderArea.offset = { 0, 0 };
    rpInfo.renderArea.extent = { cxt.extents.x, cxt.extents.y };

    vkCmdBeginRenderPass(cxt.cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void DeferredPbrRenderPass::end(RenderPassContext& cxt) {
    vkCmdEndRenderPass(cxt.cmd);
}

VkRenderPass DeferredPbrRenderPass::createVkRenderPass() {
    return VK_NULL_HANDLE;
}

std::unique_ptr<GpuImageView> DeferredPbrRenderPass::createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2& extents) {
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = getColorFormatAndSpace().getColorFormat();
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    imageCreateInfo.extent.width = extents.x;
    imageCreateInfo.extent.height = extents.y;
    imageCreateInfo.extent.depth = 1;

    VmaAllocationCreateInfo  allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

    VkImage vkImage;
    VmaAllocation vmaImageAllocation;
    VmaAllocationInfo allocationInfo;
    VKCHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocationCreateInfo, &vkImage, &vmaImageAllocation, &allocationInfo),
        "Failed to create depth stencil image.");

    auto depthImage = std::make_shared<GpuImage>(GpuImage{
            getVkDevice(),
            vmaAllocator,
            vkImage,
            vmaImageAllocation
        });

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = getColorFormatAndSpace().getColorFormat();
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.image = depthImage->vkImage;

    VkImageView vkImageView;
    VKCHECK(vkCreateImageView(getVkDevice(), &imageViewCreateInfo, VK_NULL_HANDLE, &vkImageView),
        "Failed to create depth stencil image view.");

    return std::make_unique<GpuImageView>(GpuImageView{
            depthImage,
            vkImageView
        });
}

std::unique_ptr<RenderTarget> DeferredPbrRenderPass::createRenderTarget(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents, const int renderTargetIndex) {
    return std::unique_ptr<RenderTarget>();
}

void DeferredPbrRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) {
    freeRenderTargets();

    setDepthStencil(createDepthStencil(vmaAllocator, extents));

    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {

    }
}