#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <shared_mutex>
#include <unordered_map>
#include <memory>

class VulkanContext;

class CachedGpuBuffer {
public:
    CachedGpuBuffer(uint32_t id, std::unique_ptr<GpuBuffer>&& gpuBuffer, VkDeviceSize frameSize, VkDeviceSize totalSize);

    GpuBuffer& getGpuBuffer() const {
        return *(gpuBuffer.get());
    }

    VkDeviceSize getFrameSize() const {
        return frameSize;
    }

    VkDeviceSize getTotalSize() const {
        return totalSize;
    }

    VkDeviceSize getFrameOffset(size_t frame) const {
        return frameSize * frame;
    }

    uint32_t getId() const {
        return id;
    }

private:
    const uint32_t id;
    std::unique_ptr<GpuBuffer> gpuBuffer;
    const VkDeviceSize frameSize;
    const VkDeviceSize totalSize;
};

class GpuBufferCache {

private:
    std::atomic<uint32_t> runningId;
    const VulkanContext& vkContext;
    std::unordered_map<uint32_t, std::unique_ptr<CachedGpuBuffer>> cache;
    std::shared_mutex mtx{};

public:
    GpuBufferCache(const VulkanContext& vkContext)
        : vkContext(vkContext), runningId(0) {}

    CachedGpuBuffer* get(uint32_t cacheKey);
    CachedGpuBuffer& createHostMapped(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
    CachedGpuBuffer& createHostMapped(VkDeviceSize frameSize, uint32_t frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
    CachedGpuBuffer& create(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
    CachedGpuBuffer& create(VkDeviceSize frameSize, uint32_t frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
};