#include <kengine/vulkan/GpuBufferCache.hpp>

CachedGpuBuffer* GpuBufferCache::get(unsigned int cacheKey) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = cache.find(cacheKey);

    if (it == cache.end())
        return nullptr;

    return it->second.get();
}

CachedGpuBuffer& GpuBufferCache::createHostMapped(VkDeviceSize totalSize, int usageFlags, int memoryUsage, int allocFlags) {
    return createHostMapped(totalSize, 1, usageFlags, memoryUsage, allocFlags);
}

CachedGpuBuffer& GpuBufferCache::createHostMapped(VkDeviceSize frameSize, int frameCount, int usageFlags, int memoryUsage, int allocFlags) {
    auto& buf = create(frameSize, frameCount, usageFlags, memoryUsage, allocFlags);
    buf.getGpuBuffer().map();
    return buf;
}

CachedGpuBuffer& GpuBufferCache::create(VkDeviceSize totalSize, int usageFlags, int memoryUsage, int allocFlags) {
    return create(totalSize, 1, usageFlags, memoryUsage, allocFlags);
}

CachedGpuBuffer& GpuBufferCache::create(VkDeviceSize frameSize, int frameCount, int usageFlags, int memoryUsage, int allocFlags) {
    if ((usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0) {
        frameSize = vkContext.alignUboFrame(frameSize);
    }
    else if ((usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0) {
        frameSize = vkContext.alignSsboFrame(frameSize);
    }

    return CachedGpuBuffer(runningId++, nullptr, 0,0);
}