#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <mutex>
#include <unordered_map>

class CachedGpuBuffer {
public:
    CachedGpuBuffer(int id, std::unique_ptr<GpuBuffer> gpuBuffer, VkDeviceSize frameSize, VkDeviceSize totalSize)
        : id(id), gpuBuffer(std::move(gpuBuffer)), frameSize(frameSize), totalSize(totalSize) {}

    GpuBuffer& getGpuBuffer() {
        return *(gpuBuffer.get());
    }

    VkDeviceSize getFrameOffset(size_t frame) {
        return frameSize * frame;
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
    std::mutex mtx;

public:
    GpuBufferCache(const VulkanContext& vkContext)
        : vkContext(vkContext), runningId(0) {}

     CachedGpuBuffer* get(unsigned int cacheKey);
     CachedGpuBuffer& createHostMapped(VkDeviceSize totalSize, int usageFlags, int memoryUsage, int allocFlags);
     CachedGpuBuffer& createHostMapped(VkDeviceSize frameSize, int frameCount, int usageFlags, int memoryUsage, int allocFlags);
     CachedGpuBuffer& create(VkDeviceSize totalSize, int usageFlags, int memoryUsage, int allocFlags);
     CachedGpuBuffer& create(VkDeviceSize frameSize, int frameCount, int usageFlags, int memoryUsage, int allocFlags);
};