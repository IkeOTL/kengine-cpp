#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/VulkanContext.hpp>

CachedGpuBuffer::CachedGpuBuffer(int id, std::unique_ptr<GpuBuffer>&& gpuBuffer, VkDeviceSize frameSize, VkDeviceSize totalSize)
    : id(id), gpuBuffer(std::move(gpuBuffer)), frameSize(frameSize), totalSize(totalSize) {}

CachedGpuBuffer* GpuBufferCache::get(unsigned int cacheKey) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = cache.find(cacheKey);

    if (it == cache.end())
        return nullptr;

    return it->second.get();
}

CachedGpuBuffer& GpuBufferCache::createHostMapped(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
    return createHostMapped(totalSize, 1, usageFlags, memoryUsage, allocFlags);
}

CachedGpuBuffer& GpuBufferCache::createHostMapped(VkDeviceSize frameSize, int frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
    auto& buf = create(frameSize, frameCount, usageFlags, memoryUsage, allocFlags);
    buf.getGpuBuffer().map();
    return buf;
}

CachedGpuBuffer& GpuBufferCache::create(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
    return create(totalSize, 1, usageFlags, memoryUsage, allocFlags);
}

CachedGpuBuffer& GpuBufferCache::create(VkDeviceSize frameSize, int frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
    if (usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        frameSize = vkContext.alignUboFrame(frameSize);
    else if (usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        frameSize = vkContext.alignSsboFrame(frameSize);

    auto totalSize = frameSize * frameCount;

    // get this buffer from vkcxt
    auto gpuBuf = vkContext.createBuffer(totalSize, usageFlags, memoryUsage, allocFlags);
    auto newId = runningId.fetch_add(1);
    auto buf = std::make_unique<CachedGpuBuffer>(newId, std::move(gpuBuf), frameSize, totalSize);

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache[newId] = std::move(buf);
    }

    return *(cache[newId]);
}