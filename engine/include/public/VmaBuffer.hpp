#pragma once
#include "VmaInclude.hpp"

class VmaBuffer {

private:
    const VmaAllocator vmaAllocator;
    const VkBuffer vkBuffer;
    const VmaAllocation vmaAllocation;
    const bool hostCoherent;
    void* mappedBuffer = nullptr;

public:
    VmaBuffer(
        VmaAllocator vmaAllocator,
        VkBuffer vkBuffer,
        VmaAllocation vmaAllocation,
        bool hostCoherent
    )
        : vmaAllocator(vmaAllocator),
        vkBuffer(vkBuffer),
        vmaAllocation(vmaAllocation),
        hostCoherent(hostCoherent)
    { }

    ~VmaBuffer();

    VkBuffer getVkBuffer() {
        return vkBuffer;
    }

    void* data();
    void* map();
    void unmap();
    void flush(unsigned long offset, unsigned long size);
};