#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <glm/vec2.hpp>

// render target
VkFramebuffer DeferredPbrRenderTarget::createFramebuffer(RenderPass& renderPass, VmaAllocator vmaAllocator,
    const std::vector<VkImageView>& sharedImageViews, const glm::uvec2& extents) {
    VkImageUsageFlags rpUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    // Create attachment images
    albedoImage = createAttachmentImage(vmaAllocator, VK_FORMAT_R8G8B8A8_SRGB, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
    positionImage = createAttachmentImage(vmaAllocator, VK_FORMAT_R32G32B32A32_SFLOAT, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
    normalImage = createAttachmentImage(vmaAllocator, VK_FORMAT_R16G16_SNORM, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
    ormImage = createAttachmentImage(vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);
    emissiveImage = createAttachmentImage(vmaAllocator, VK_FORMAT_R8G8B8A8_SRGB, rpUsage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VK_IMAGE_ASPECT_COLOR_BIT, extents);

    std::vector<VkImageView> attachments = {
        sharedImageViews[0],
        albedoImage->imageView,
        positionImage->imageView,
        normalImage->imageView,
        ormImage->imageView,
        emissiveImage->imageView,
        renderPass.getDepthStencilImageView().imageView
    };

    // Add your depth stencil image view to the attachments list as needed

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.getVkRenderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extents.x;
    framebufferInfo.height = extents.y;
    framebufferInfo.layers = 1;

    VkFramebuffer framebuffer;
    VKCHECK(vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &framebuffer),
        "Failed to create framebuffer");

    return framebuffer;
}

std::unique_ptr<GpuImageView>&& DeferredPbrRenderTarget::createAttachmentImage(VmaAllocator vmaAllocator, VkFormat format, VkImageUsageFlags imageUsage,
    VmaMemoryUsage memUsage, VkImageAspectFlags viewAspectMask, const glm::uvec2 extents) {
    VkImageCreateInfo imageCreateInfo{};
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

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = memUsage;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
        allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

    VkImage vkImage;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VKCHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocCreateInfo, &vkImage, &allocation, &allocationInfo),
        "Failed to create image");

    auto gpuImage = std::make_shared<GpuImage>(GpuImage{
            vkDevice,
            vmaAllocator,
            vkImage,
            allocation,
            imageCreateInfo
        });

    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = vkImage;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = viewAspectMask;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView vkImageView;
    VKCHECK(vkCreateImageView(vkDevice, &viewCreateInfo, nullptr, &vkImageView),
        "Failed to create image view");

    return std::make_unique<GpuImageView>(GpuImageView{
            gpuImage,
            vkImageView,
            viewCreateInfo
        });
}

const GpuImageView& DeferredPbrRenderTarget::getAlbedoImage() const {
    if (!albedoImage)
        throw std::runtime_error("Albedo image missing.");

    return *albedoImage;
}

const GpuImageView& DeferredPbrRenderTarget::getPositionImage() const {
    if (!positionImage)
        throw std::runtime_error("Position image missing.");

    return *positionImage;
}

const GpuImageView& DeferredPbrRenderTarget::getNormalImage() const {
    if (!normalImage)
        throw std::runtime_error("Normal image missing.");

    return *normalImage;
}

const GpuImageView& DeferredPbrRenderTarget::getOrmImage() const {
    if (!ormImage)
        throw std::runtime_error("ORM image missing.");

    return *ormImage;
}

const GpuImageView& DeferredPbrRenderTarget::getEmissiveImage() const {
    if (!emissiveImage)
        throw std::runtime_error("Emissive image missing.");

    return *emissiveImage;
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

    rpInfo.framebuffer = getRenderTarget(cxt.renderTargetIndex).getVkFramebuffer();

    rpInfo.renderArea.offset = { 0, 0 };
    rpInfo.renderArea.extent = { cxt.extents.x, cxt.extents.y };

    vkCmdBeginRenderPass(cxt.cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void DeferredPbrRenderPass::end(RenderPassContext& cxt) {
    vkCmdEndRenderPass(cxt.cmd);
}

VkRenderPass DeferredPbrRenderPass::createVkRenderPass() {
    std::vector<VkAttachmentDescription> attachments = {
        { // Swapchain color attachment
            0, // flags
            colorFormatAndSpace.getColorFormat(),// format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_STORE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // finalLayout
        },
        { // albedo
            0, // flags
            VK_FORMAT_R8G8B8A8_SRGB,// format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // finalLayout
        },
        { // position
            0, // flags
            VK_FORMAT_R32G32B32A32_SFLOAT,// format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // finalLayout
        },
        { // normal
            0, // flags
            VK_FORMAT_R16G16_SNORM,// format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // finalLayout
        },
        { // orm
            0, // flags
            VK_FORMAT_R16G16B16A16_SFLOAT,// format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // finalLayout
        },
        { // emissive
            0, // flags
            VK_FORMAT_R8G8B8A8_SRGB,// format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // finalLayout
        },
        { // Depth attachment
            0, // flags
            colorFormatAndSpace.getDepthFormat(), // format
            VK_SAMPLE_COUNT_1_BIT, // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR, // loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED, // initialLayout
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // finalLayout
        }
    };

    VkAttachmentReference depthReference = {
        6, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // Define subpasses
    std::vector<VkSubpassDescription> subpasses(4);

    // subpass 1
    std::vector<VkAttachmentReference> subpass1_colorReferences = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
    };

    subpasses[0] = {
        0, // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
        0, // inputAttachmentCount
        nullptr, // pInputAttachments
        static_cast<uint32_t>(subpass1_colorReferences.size()), // colorAttachmentCount
        subpass1_colorReferences.data(), // pColorAttachments
        nullptr, // pResolveAttachments
        &depthReference, // pDepthStencilAttachment
        0, // preserveAttachmentCount
        nullptr // pPreserveAttachments
    };

    // subpass 2    
    std::vector<VkAttachmentReference> subpass2_colorReferences = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    std::vector<VkAttachmentReference> subpass2_inputRefs = {
        {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        {4, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        {5, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
    };

    subpasses[1] = {
        0, // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
        static_cast<uint32_t>(subpass2_inputRefs.size()), // inputAttachmentCount
        subpass2_inputRefs.data(), // pInputAttachments
        static_cast<uint32_t>(subpass2_colorReferences.size()), // colorAttachmentCount
        subpass2_colorReferences.data(), // pColorAttachments
        nullptr, // pResolveAttachments
        &depthReference, // pDepthStencilAttachment
        0, // preserveAttachmentCount
        nullptr // pPreserveAttachments
    };


    // subpass 3    
    std::vector<VkAttachmentReference> subpass3_colorReferences = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
    };

    std::vector<VkAttachmentReference> subpass3_inputRefs = {
        {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
    };

    subpasses[2] = {
        0, // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
        static_cast<uint32_t>(subpass3_colorReferences.size()), // inputAttachmentCount
        subpass3_colorReferences.data(), // pInputAttachments
        static_cast<uint32_t>(subpass3_colorReferences.size()), // colorAttachmentCount
        subpass3_colorReferences.data(), // pColorAttachments
        nullptr, // pResolveAttachments
        &depthReference, // pDepthStencilAttachment
        0, // preserveAttachmentCount
        nullptr // pPreserveAttachments
    };

    // subpass 4 GUI    
    std::vector<VkAttachmentReference> subpass4_colorReferences = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
    };

    subpasses[3] = {
        0, // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
        0, // inputAttachmentCount
        nullptr, // pInputAttachments
        static_cast<uint32_t>(subpass4_colorReferences.size()), // colorAttachmentCount
        subpass4_colorReferences.data(), // pColorAttachments
        nullptr, // pResolveAttachments
        nullptr, // pDepthStencilAttachment
        0, // preserveAttachmentCount
        nullptr // pPreserveAttachments
    };

    // Subpass dependencies 
    std::vector<VkSubpassDependency> dependencies = {
        {
            VK_SUBPASS_EXTERNAL, // srcSubpass
            0, // dstSubpass
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // srcStageMask
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // dstStageMask
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, // srcAccessMask
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT // dependencyFlags
        },
        {
            VK_SUBPASS_EXTERNAL, // srcSubpass
            0, // dstSubpass
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
            VK_ACCESS_MEMORY_READ_BIT, // srcAccessMask
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT // dependencyFlags
        },
        {
            0, // srcSubpass
            1, // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
            VK_ACCESS_SHADER_READ_BIT, // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT // dependencyFlags
        },
        {
            1, // srcSubpass
            2, // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
            VK_ACCESS_SHADER_READ_BIT, // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT // dependencyFlags
        },
        {
            2, // srcSubpass
            3, // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
            VK_ACCESS_SHADER_READ_BIT, // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT // dependencyFlags
        },
        {
            3, // srcSubpass
            VK_SUBPASS_EXTERNAL, // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT, // dstAccessMask
            VK_DEPENDENCY_BY_REGION_BIT // dependencyFlags
        }
    };

    // Create the render pass
    VkRenderPassCreateInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,      // sType
        nullptr,                                        // pNext
        0,                                              // flags
        static_cast<uint32_t>(attachments.size()),      // attachmentCount
        attachments.data(),                             // pAttachments
        static_cast<uint32_t>(subpasses.size()),        // subpassCount
        subpasses.data(),                               // pSubpasses
        static_cast<uint32_t>(dependencies.size()),     // dependencyCount
        dependencies.data()                             // pDependencies
    };

    VkRenderPass renderPass;
    VKCHECK(vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &renderPass),
        "Failed to create render pass");

    return renderPass;
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
            vmaImageAllocation,
            imageCreateInfo
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
            vkImageView,
            imageViewCreateInfo
        });
}

void DeferredPbrRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView> sharedImageViews, const glm::uvec2 extents) {
    freeRenderTargets();
    setDepthStencil(createDepthStencil(vmaAllocator, extents));
    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto fb = std::make_unique<DeferredPbrRenderTarget>(vkDevice);
        fb->init(*this, vmaAllocator, { sharedImageViews[i] }, extents);
        addRenderTarget(std::move(fb));
    }
}