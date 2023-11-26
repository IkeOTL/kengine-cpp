#pragma once
#include "VmaInclude.hpp"

class GpuBuffer {

private:
    const VmaAllocator vmaAllocator;
    const VkBuffer vkBuffer;
    const VmaAllocation vmaAllocation;
    const bool hostCoherent;
    void* mappedBuffer = nullptr;

public:
    GpuBuffer(
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

    ~GpuBuffer();

    VkBuffer getVkBuffer() {
        return vkBuffer;
    }

    void* data();
    void* map();
    void unmap();
    void flush(unsigned long offset, unsigned long size);
};