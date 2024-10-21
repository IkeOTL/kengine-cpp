#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/Logger.hpp>

#include <stdexcept>


GpuBuffer::GpuBuffer(
    uint32_t id,
    VmaAllocator vmaAllocator,
    VkBuffer vkBuffer,
    VmaAllocation vmaAllocation,
    bool hostCoherent
)
    : id(id),
    vmaAllocator(vmaAllocator),
    vkBuffer(vkBuffer),
    vmaAllocation(vmaAllocation),
    hostCoherent(hostCoherent)
{
    KE_LOG_DEBUG(std::format("GpuBuffer created: {}", id));
}

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
    if (hostCoherent)
        return;

    vmaFlushAllocation(vmaAllocator, vmaAllocation, offset, size);
}

GpuBuffer::~GpuBuffer() {
    if (mappedBuffer) {
        vmaUnmapMemory(vmaAllocator, vmaAllocation);
        mappedBuffer = nullptr;
        KE_LOG_DEBUG(std::format("GpuBuffer unmapped: {}", id));
    }

    vmaDestroyBuffer(vmaAllocator, vkBuffer, vmaAllocation);
    KE_LOG_DEBUG(std::format("GpuBuffer destroyed: {}", id));
};