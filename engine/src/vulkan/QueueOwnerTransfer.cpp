#include <kengine/vulkan/QueueOwnerTransfer.hpp>

void BufferQueueOwnerTransfer::applyReleaseBarrier(VkCommandBuffer cmd) {
    if (srcQueueFamily == dstQueueFamily)
        return;

    VkBufferMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
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
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
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
    depsInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depsInfo);
}

void ImageQueueOwnerTransfer::applyAcquireBarrier(VkCommandBuffer cmd) {
    if (srcQueueFamily == dstQueueFamily)
        return;

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    barrier.srcAccessMask = 0;
    barrier.dstStageMask = dstStageMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
    depsInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depsInfo);

    generateMipmaps(cmd);
}


void ImageQueueOwnerTransfer::generateMipmaps(VkCommandBuffer cmd) {
    auto mipWidth = texWidth;
    auto mipHeight = texHeight;

    for (uint32_t level = 1; level < mipLevels; level++) {
        {
            VkImageMemoryBarrier2 barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = vkImage;
            barrier.subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                level - 1,
                1,
                0,
                1
            };

            VkDependencyInfo depsInfo{};
            depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depsInfo.pImageMemoryBarriers = &barrier;

            vkCmdPipelineBarrier2(cmd, &depsInfo);
        }

        // blit
        {
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                level - 1,
                0,
                1
            };

            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                level,
                0,
                1
            };

            vkCmdBlitImage(
                cmd,
                vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);
        }

        {
            VkImageMemoryBarrier2 barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = vkImage;
            barrier.subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                level - 1,
                1,
                0,
                1
            };

            VkDependencyInfo depsInfo{};
            depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depsInfo.pImageMemoryBarriers = &barrier;

            vkCmdPipelineBarrier2(cmd, &depsInfo);
        }

        if (mipWidth > 1)
            mipWidth /= 2;

        if (mipHeight > 1)
            mipHeight /= 2;
    }

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = vkImage;
    barrier.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        mipLevels - 1,
        1,
        0,
        1
    };

    VkDependencyInfo depsInfo{};
    depsInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depsInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depsInfo);
}