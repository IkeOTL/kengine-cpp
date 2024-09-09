#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <mutex>

class QueueOwnerTransfer {

protected:
    const uint32_t srcQueueFamily;
    const uint32_t dstQueueFamily;
    const VkAccessFlags2 dstStageMask;
    const VkAccessFlags2 dstAccessMask;

public:
    QueueOwnerTransfer(uint32_t srcQueueFamily, uint32_t dstQueueFamily, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
        : srcQueueFamily(srcQueueFamily), dstQueueFamily(dstQueueFamily), dstStageMask(dstStageMask), dstAccessMask(dstAccessMask) {}

    virtual ~QueueOwnerTransfer() = default;

    virtual void applyReleaseBarrier(VkCommandBuffer cmd) = 0;
    virtual void applyAcquireBarrier(VkCommandBuffer cmd) = 0;
};

class BufferQueueOwnerTransfer : public QueueOwnerTransfer {
private:
    const VkBuffer vkBuffer;
    const VkDeviceSize bufSize;

public:
    BufferQueueOwnerTransfer(VkBuffer vkBuffer, VkDeviceSize bufSize, uint32_t srcQueueFamily, uint32_t dstQueueFamily, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
        : QueueOwnerTransfer(srcQueueFamily, dstQueueFamily, dstStageMask, dstAccessMask),
        vkBuffer(vkBuffer), bufSize(bufSize)
    {}

    void applyReleaseBarrier(VkCommandBuffer cmd) override;
    void applyAcquireBarrier(VkCommandBuffer cmd) override;
};

class ImageQueueOwnerTransfer : public QueueOwnerTransfer {
private:
    const VkImage vkImage;
    uint32_t mipLevels;
    int32_t texWidth, texHeight;

    void generateMipmaps(VkCommandBuffer cmd);

public:
    ImageQueueOwnerTransfer(VkImage vkImage, uint32_t srcQueueFamily, uint32_t dstQueueFamily, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
        : QueueOwnerTransfer(srcQueueFamily, dstQueueFamily, dstStageMask, dstAccessMask),
        vkImage(vkImage), mipLevels(0), texWidth(0), texHeight(0)
    {}

    void setMips(uint32_t mipLevels, int32_t texWidth, int32_t texHeight);
    void applyReleaseBarrier(VkCommandBuffer cmd) override;
    void applyAcquireBarrier(VkCommandBuffer cmd) override;
};