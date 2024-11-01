#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/VulkanContext.hpp>

namespace ke {
    CachedGpuBuffer::CachedGpuBuffer(GpuBufferId id, std::unique_ptr<GpuBuffer>&& gpuBuffer, VkDeviceSize frameSize, VkDeviceSize totalSize)
        : id(id),
          gpuBuffer(std::move(gpuBuffer)),
          frameSize(frameSize),
          totalSize(totalSize) {}

    CachedGpuBuffer* GpuBufferCache::get(GpuBufferId cacheKey) {
        std::shared_lock<std::shared_mutex> lock(this->mtx);
        auto it = cache.find(cacheKey);

        if (it == cache.end())
            return nullptr;

        return it->second.get();
    }

    CachedGpuBuffer& GpuBufferCache::createHostMapped(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
        return createHostMapped(totalSize, 1, usageFlags, memoryUsage, allocFlags);
    }

    CachedGpuBuffer& GpuBufferCache::createHostMapped(VkDeviceSize frameSize, uint32_t frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
        auto& buf = create(frameSize, frameCount, usageFlags, memoryUsage, allocFlags);
        buf.getGpuBuffer().map();
        return buf;
    }

    CachedGpuBuffer& GpuBufferCache::create(VkDeviceSize totalSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
        return create(totalSize, 1, usageFlags, memoryUsage, allocFlags);
    }

    CachedGpuBuffer& GpuBufferCache::create(VkDeviceSize frameSize, uint32_t frameCount, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) {
        // only align if we have frames? or align everytgin just in case
        // if (frameCount != 0)
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
            std::unique_lock<std::shared_mutex> lock(this->mtx);
            cache[newId] = std::move(buf);
        }

        return *(cache[newId]);
    }

    CachedGpuBuffer& GpuBufferCache::upload(std::function<void(VulkanContext& vkCxt, void* data, VkDeviceSize frameSize, uint32_t frameCount)> dataProvider,
        VkDeviceSize frameSize, uint32_t frameCount,
        VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
        VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocFlags) {
        // only align if we have frames? or align everytgin just in case
        // if (frameCount != 0)
        if (usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            frameSize = vkContext.alignUboFrame(frameSize);
        else if (usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            frameSize = vkContext.alignSsboFrame(frameSize);

        auto totalSize = frameSize * frameCount;

        auto gpuBuf = vkContext.uploadBuffer(
            [&dataProvider, frameSize, frameCount](VulkanContext& vkCxt, void* data) {
                dataProvider(vkCxt, data, frameSize, frameCount);
            },
            totalSize,
            dstStageMask, dstAccessMask,
            usageFlags, allocFlags, nullptr);

        auto newId = runningId.fetch_add(1);
        auto buf = std::make_unique<CachedGpuBuffer>(newId, std::move(gpuBuf), frameSize, totalSize);

        {
            std::unique_lock<std::shared_mutex> lock(this->mtx);
            cache[newId] = std::move(buf);
        }

        return *(cache[newId]);
    }
} // namespace ke