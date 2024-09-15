#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <shared_mutex>
#include <unordered_map>

class VulkanContext;

class CachedGpuBuffer {
public:
    CachedGpuBuffer(int id, std::unique_ptr<GpuBuffer>&& gpuBuffer, VkDeviceSize frameSize, VkDeviceSize totalSize);

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

    unsigned int getId() const {
        return id;
    }

private:
    const unsigned int id;
    std::unique_ptr<GpuBuffer> gpuBuffer;
    const VkDeviceSize frameSize;
    const VkDeviceSize totalSize;
};

class GpuBufferCache {

private:
    std::atomic<unsigned int> runningId;
    const VulkanContext& vkContext;
    std::unordered_map<unsigned int, std::unique_ptr<CachedGpuBuffer>> cache;
    std::shared_mutex mtx{};

public:
    GpuBufferCache(const VulkanContext& vkContext)
        : vkContext(vkContext), runningId(0) {}

    CachedGpuBuffer* get(unsigned int cacheKey);
    CachedGpuBuffer& createHostMapped(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
    CachedGpuBuffer& createHostMapped(VkDeviceSize frameSize, int frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
    CachedGpuBuffer& create(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
    CachedGpuBuffer& create(VkDeviceSize frameSize, int frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags);
};