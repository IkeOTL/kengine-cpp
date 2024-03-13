#include <kengine/vulkan/texture/Texture2d.hpp>

void Texture2d::init(VulkanContext& vkCxt, const unsigned char* image, uint32_t width, uint32_t height,
    VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels,
    VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, bool generateMipMaps)
{
    this->width = width;
    this->height = height;

    // Calculate mip levels
    mipLevels = !generateMipMaps ? 1 : static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;

    size_t imageSize = width * height * channels;

    // Create staging buffer
    auto xferFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    auto stagingBuffer = vkCxt.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, xferFlags);

    // Copy data to staging buffer
    {
        GpuBuffer::ScopedMap scopedMap(*stagingBuffer);
        memcpy(stagingBuffer->data(), image, imageSize);
    }

    // Create image
    VkExtent3D imageExtent = { width, height, 1 };

    VkImageCreateInfo imgCreateInfo{};
    imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgCreateInfo.imageType = imageType;
    imgCreateInfo.format = format;
    imgCreateInfo.mipLevels = mipLevels;
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imgCreateInfo.extent = imageExtent;

    VmaAllocationCreateInfo allocImgInfo{};
    allocImgInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkImage imageHandle;
    VmaAllocation imageAllocation;
    vmaCreateImage(vkCxt.getVmaAllocator(), &imgCreateInfo, &allocImgInfo, &imageHandle, &imageAllocation, nullptr);

    gpuImage = std::make_shared<GpuImage>(
        vkCxt.getVkDevice(),
        vkCxt.getVmaAllocator(),
        imageHandle,
        imageAllocation,
        imgCreateInfo
    );

    // Transition image to transfer-receiver
    std::shared_ptr<GpuBuffer> sStagingBuf = std::move(stagingBuffer);
    vkCxt.recordAndSubmitTransferCmdBuf([&](const CommandBuffer& cmdBuf) {
        auto qXfer = std::make_shared<ImageQueueOwnerTransfer>(
            imageHandle, vkCxt.getXferQueueFamilyIndex(), vkCxt.getGfxQueueFamilyIndex(), dstStageMask, dstAccessMask
        );

        qXfer->setMips(mipLevels, width, height);

        VkImageMemoryBarrier imageBarrierToTransfer{};
        imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierToTransfer.image = imageHandle;
        imageBarrierToTransfer.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
        imageBarrierToTransfer.srcAccessMask = 0;
        imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(cmdBuf.vkCmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.imageExtent = imageExtent;

        vkCmdCopyBufferToImage(cmdBuf.vkCmdBuf, sStagingBuf->getVkBuffer(), imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        qXfer->applyReleaseBarrier(cmdBuf.vkCmdBuf);

        return [&vkCxt, sStagingBuf, qXfer]() {
            vkCxt.submitQueueTransfer(qXfer);
            };
        }, true);

    // Create image view
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = imageHandle;
    viewCreateInfo.viewType = imageViewType;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 };

    VkImageView imageView;
    VKCHECK(vkCreateImageView(vkCxt.getVkDevice(), &viewCreateInfo, nullptr, &imageView),
        "Failed to create image view");

    gpuImageView = std::make_unique<GpuImageView>(
        gpuImage,
        imageView,
        viewCreateInfo
    );
}
