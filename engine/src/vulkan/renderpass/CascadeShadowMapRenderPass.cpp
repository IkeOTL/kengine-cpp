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
    for (uint32_t i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {
        auto fb = std::make_unique<CascadeShadowMapRenderTarget>(vkDevice, *getDepthStencil(), i);
        fb->init(*this, vmaAllocator, {}, extents);
        addRenderTarget(std::move(fb));
    }
}

// render pass
void CascadeShadowMapRenderPass::begin(RenderPassContext& cxt) {
    VkClearValue clearValues{};
    clearValues.depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = getVkRenderPass();
    renderPassInfo.framebuffer = getRenderTarget(cxt.renderTargetIndex).getVkFramebuffer();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValues;

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { cxt.extents.x, cxt.extents.y };

    vkCmdBeginRenderPass(cxt.cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CascadeShadowMapRenderPass::end(RenderPassContext& cxt) {
    vkCmdEndRenderPass(cxt.cmd);
}

VkRenderPass CascadeShadowMapRenderPass::createVkRenderPass() {
    VkAttachmentDescription attachments{};
    attachments.format = colorFormatAndSpace.getDepthFormat(); // Adapt this to your actual format retrieval
    attachments.samples = VK_SAMPLE_COUNT_1_BIT;
    attachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthReference{};
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &depthReference;

    VkSubpassDependency dependencies[2]{};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies;

    VkRenderPass renderPass;
    VKCHECK(vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &renderPass),
        "Failed to create clear render pass");

    return renderPass;
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