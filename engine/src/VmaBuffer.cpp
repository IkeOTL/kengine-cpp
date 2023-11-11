#include "VmaBuffer.hpp"
#include <stdexcept>

void* VmaBuffer::data() {
    if (!mappedBuffer)
        throw std::runtime_error("Buffer not mapped.");

    return mappedBuffer;
}

void* VmaBuffer::map() {
    if (mappedBuffer)
        return mappedBuffer;

    vmaMapMemory(vmaAllocator, vmaAllocation, &mappedBuffer);

    return data();
}

void VmaBuffer::unmap() {
    if (!mappedBuffer)
        return;

    vmaUnmapMemory(vmaAllocator, vmaAllocation);
    mappedBuffer = nullptr;
}

void VmaBuffer::flush(unsigned long offset, unsigned long size) {
    if (!hostCoherent)
        return;

    vmaFlushAllocation(vmaAllocator, vmaAllocation, offset, size);
}

VmaBuffer::~VmaBuffer() {
    if (!mappedBuffer) {
        vmaUnmapMemory(vmaAllocator, vmaAllocation);
        mappedBuffer = nullptr;
    }

    vmaDestroyBuffer(vmaAllocator, vkBuffer, vmaAllocation);
};