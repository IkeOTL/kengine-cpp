#include <kengine/vulkan/GpuBuffer.hpp>
#include <stdexcept>

void* GpuBuffer::data() {
    if (!mappedBuffer)
        throw std::runtime_error("Buffer not mapped.");

    return mappedBuffer;
}

void* GpuBuffer::map() {
    if (mappedBuffer)
        return mappedBuffer;

    vmaMapMemory(vmaAllocator, vmaAllocation, &mappedBuffer);

    return data();
}

void GpuBuffer::unmap() {
    if (!mappedBuffer)
        return;

    vmaUnmapMemory(vmaAllocator, vmaAllocation);
    mappedBuffer = nullptr;
}

void GpuBuffer::flush(unsigned long offset, unsigned long size) {
    if (!hostCoherent)
        return;

    vmaFlushAllocation(vmaAllocator, vmaAllocation, offset, size);
}

GpuBuffer::~GpuBuffer() {
    if (!mappedBuffer) {
        vmaUnmapMemory(vmaAllocator, vmaAllocation);
        mappedBuffer = nullptr;
    }

    vmaDestroyBuffer(vmaAllocator, vkBuffer, vmaAllocation);
};