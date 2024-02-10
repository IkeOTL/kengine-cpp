#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>

// render target
review
VkFramebuffer DeferredPbrRenderTarget::createFramebuffer(RenderPass& renderPass, VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) {
    VkImageUsageFlags rpUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    // Create attachment images
     albedoImage = createAttachmentImage(VK_FORMAT_R8G8B8A8_SRGB, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
     positionImage = createAttachmentImage(VK_FORMAT_R32G32B32A32_SFLOAT, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
     normalImage = createAttachmentImage(VK_FORMAT_R16G16_SNORM, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
     ormImage = createAttachmentImage(VK_FORMAT_R16G16B16A16_SFLOAT, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
     emissiveImage = createAttachmentImage(VK_FORMAT_R8G8B8A8_SRGB, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);

    std::vector<VkImageView> attachments = {
        sharedImageViews[0],
        albedoImage,
        positionImage,
        normalImage,
        ormImage,
        emissiveImage,
        renderPass.getDepthStencilImageView().getImageView()
    };

    // Add your depth stencil image view to the attachments list as needed

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extents.x;
    framebufferInfo.height = extents.y;
    framebufferInfo.layers = 1;

    VkFramebuffer framebuffer;
    VKCHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer),
        "Failed to create framebuffer");

    return framebuffer;
}

review
std::unique_ptr<GpuImageView>&& DeferredPbrRenderTarget::createAttachmentImage(VkFormat format, VkImageUsageFlags imageUsage,
    VmaMemoryUsage memUsage, VkImageAspectFlags viewAspectMask, const glm::uvec2 extents) {
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = imageUsage;
    imageCreateInfo.extent.width = extents.x;
    imageCreateInfo.extent.height = extents.y;
    imageCreateInfo.extent.depth = 1;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = memUsage;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VKCHECK(vmaCreateImage(allocator, &imageCreateInfo, &allocCreateInfo, &image, &allocation, &allocationInfo), "Failed to create image");

    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = viewAspectMask;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VKCHECK(vkCreateImageView(device, &viewCreateInfo, nullptr, &imageView), "Failed to create image view");

    return unique_ptr<GpuImageView>(imageView)
}

// render pass
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

std::unique_ptr<GpuImageView> DeferredPbrRenderPass::createDepthStencil(VmaAllocator vmaAllocator, const glm::uvec2 extents) {
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

void DeferredPbrRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) {
    freeRenderTargets();

    setDepthStencil(createDepthStencil(vmaAllocator, extents));

    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {

    }
}