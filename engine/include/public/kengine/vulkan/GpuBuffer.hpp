#pragma once
#include "VmaInclude.hpp"

class GpuBuffer {
private:
    const VmaAllocator vmaAllocator;
    const VmaAllocation vmaAllocation;
    const bool hostCoherent;
    void* mappedBuffer = nullptr;

public:
    const VkBuffer vkBuffer;

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

    VkBuffer getVkBuffer() const {
        return vkBuffer;
    }

    void* data();
    void* map();
    void unmap();
    void flush(unsigned long offset, unsigned long size);

    class ScopedMap {
        GpuBuffer& buf;
    public:
        ScopedMap(GpuBuffer& buffer) : buf(buffer) {
            buf.map();
        }

        ~ScopedMap() {
            buf.unmap();
        }
    };
};