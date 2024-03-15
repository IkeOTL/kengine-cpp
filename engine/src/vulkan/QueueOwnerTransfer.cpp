#include <kengine/vulkan/QueueOwnerTransfer.hpp>

void BufferQueueOwnerTransfer::applyReleaseBarrier(VkCommandBuffer cmd) {
    if (srcQueueFamily == dstQueueFamily)
        return;

    VkBufferMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrier.dstAccessMask = 0;
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;
    barrier.buffer = vkBuffer;
    barrier.offset = 0;
    barrier.size = bufSize;

    VkDependencyInfo depsInfo{};
    depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depsInfo.bufferMemoryBarrierCount = 1;
    depsInfo.pBufferMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depsInfo);
}

void BufferQueueOwnerTransfer::applyAcquireBarrier(VkCommandBuffer cmd) {
    if (srcQueueFamily == dstQueueFamily)
        return;

    VkBufferMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    barrier.srcAccessMask = 0;
    barrier.dstStageMask = dstStageMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;
    barrier.buffer = vkBuffer;
    barrier.offset = 0;
    barrier.size = bufSize;

    VkDependencyInfo depsInfo{};
    depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depsInfo.bufferMemoryBarrierCount = 1;
    depsInfo.pBufferMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depsInfo);
}

void ImageQueueOwnerTransfer::setMips(uint32_t mipLevels, int32_t texWidth, int32_t texHeight) {
    this->mipLevels = mipLevels;
    this->texWidth = texWidth;
    this->texHeight = texHeight;
}

void ImageQueueOwnerTransfer::applyReleaseBarrier(VkCommandBuffer cmd) {
    if (srcQueueFamily == dstQueueFamily)
        return;

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;
    barrier.image = vkImage;
    barrier.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        VK_REMAINING_MIP_LEVELS,
        0,
        VK_REMAINING_ARRAY_LAYERS
    };

    VkDependencyInfo depsInfo{};
    depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depsInfo.imageMemoryBarrierCount = 1;
    depsInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depsInfo);
}
//https://discord.com/channels/427551838099996672/427833316536746005/1217868728130207784
//https://discord.com/channels/427551838099996672/427833316536746005/1217868952550506556
void ImageQueueOwnerTransfer::applyAcquireBarrier(VkCommandBuffer cmd) {
    auto shouldGenerateMipmaps = mipLevels > 1;
    auto shouldTransferOwnership = srcQueueFamily != dstQueueFamily;

    // transfer ownership if needed, and transition
    {
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.srcAccessMask = 0;
        barrier.dstStageMask = dstStageMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = shouldGenerateMipmaps ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = shouldTransferOwnership ? srcQueueFamily : VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = shouldTransferOwnership ? dstQueueFamily : VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkImage;
        barrier.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, VK_REMAINING_MIP_LEVELS,
            0, VK_REMAINING_ARRAY_LAYERS
        };

        VkDependencyInfo depsInfo{};
        depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depsInfo.imageMemoryBarrierCount = 1;
        depsInfo.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(cmd, &depsInfo);
    }

    if (shouldGenerateMipmaps)
        generateMipmaps(cmd);
}

void ImageQueueOwnerTransfer::generateMipmaps(VkCommandBuffer cmd) {
    auto mipWidth = texWidth;
    auto mipHeight = texHeight;

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.image = vkImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    VkDependencyInfo depsInfo{};
    depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depsInfo.imageMemoryBarrierCount = 1;
    depsInfo.pImageMemoryBarriers = &barrier;

    for (uint32_t level = 1; level < mipLevels; level++) {
        barrier.subresourceRange.baseMipLevel = level - 1;

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vkCmdPipelineBarrier2(cmd, &depsInfo);

        // blit
        {
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                level - 1,
                0, 1
            };

            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                level,
                0,  1
            };

            vkCmdBlitImage(
                cmd,
                vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);
        }

        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = dstStageMask;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier.dstAccessMask = dstAccessMask;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier2(cmd, &depsInfo);

        if (mipWidth > 1)
            mipWidth /= 2;

        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstStageMask = dstStageMask;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier2(cmd, &depsInfo);
}