#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>
#include <kengine/vulkan/ShadowCascade.hpp>

// render target
VkFramebuffer CascadeShadowMapRenderTarget::createFramebuffer(RenderPass& renderPass, VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) {
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = shadowMapDepthImage.gpuImage->vkImage;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewCreateInfo.format = shadowMapDepthImage.gpuImage->imageInfo.format;
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = cascadeIndex;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView cascadeImageView;
    VKCHECK(vkCreateImageView(vkDevice, &viewCreateInfo, nullptr, &cascadeImageView),
        "Failed to create depth-stencil image view");

    VkFramebufferCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fci.renderPass = renderPass.getVkRenderPass(); 
    fci.attachmentCount = 1;
    VkImageView attachments[] = { cascadeImageView };
    fci.pAttachments = attachments;
    fci.width = extents.x;
    fci.height = extents.y;
    fci.layers = 1;

    VkFramebuffer framebuffer;
    VKCHECK(vkCreateFramebuffer(vkDevice, &fci, nullptr, &framebuffer),
        "Failed to create framebuffer");

    return framebuffer;
}

void CascadeShadowMapRenderPass::init(VulkanContext& vkCtx) {
    RenderPass::init(vkCtx);

    createRenderTargets(vkCtx.getVmaAllocator(), {}, glm::uvec2(4096));
}

void CascadeShadowMapRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) {
    setDepthStencil(createDepthStencil(vmaAllocator, extents));
    for (size_t i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {
        auto fb = std::make_unique<CascadeShadowMapRenderTarget>(vkDevice, *getDepthStencil(), i);
        fb->init(*this, vmaAllocator, {}, extents);
        addRenderTarget(std::move(fb));
    }
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
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = ShadowCascadeData::SHADOW_CASCADE_COUNT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.format = colorFormatAndSpace.getDepthFormat();
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.extent.width = extents.x;
    imageCreateInfo.extent.height = extents.y;
    imageCreateInfo.extent.depth = 1;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkImage vkImage;
    VmaAllocation vmaImageAllocation;
    VmaAllocationInfo allocationInfo;

    VKCHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocCreateInfo, &vkImage, &vmaImageAllocation, &allocationInfo),
        "Failed to create depth-stencil image");

    auto depthImage = std::make_shared<GpuImage>(GpuImage{
            getVkDevice(),
            vmaAllocator,
            vkImage,
            vmaImageAllocation,
            imageCreateInfo
        });

    VkImageViewCreateInfo depthViewCreateInfo{};
    depthViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthViewCreateInfo.image = vkImage;
    depthViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    depthViewCreateInfo.format = colorFormatAndSpace.getDepthFormat();
    depthViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthViewCreateInfo.subresourceRange.baseMipLevel = 0;
    depthViewCreateInfo.subresourceRange.levelCount = 1;
    depthViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    depthViewCreateInfo.subresourceRange.layerCount = ShadowCascadeData::SHADOW_CASCADE_COUNT;

    VkImageView depthImageView;
    VKCHECK(vkCreateImageView(vkDevice, &depthViewCreateInfo, nullptr, &depthImageView),
        "Failed to create depth-stencil image view");

    return std::make_unique<GpuImageView>(GpuImageView{
            depthImage,
            depthImageView,
            depthViewCreateInfo
        });
}