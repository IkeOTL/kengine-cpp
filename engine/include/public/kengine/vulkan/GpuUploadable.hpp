#pragma once
#include <kengine/vulkan/GpuBuffer.hpp>
#include <memory>

class VulkanContext;

class GpuUploadable {
private:
    std::unique_ptr<GpuBuffer> gpuBuffer;
    VkDeviceSize totalSize;

public:
    GpuUploadable(VkDeviceSize totalSize)
        : totalSize(totalSize) {}

    virtual void upload(VulkanContext& vkCxt, void* data) = 0;
    virtual VkDeviceSize size() {
        return totalSize;
    }

    void setGpuBuffer(std::unique_ptr<GpuBuffer>&& buf) {
        gpuBuffer = std::move(buf);
    }

    GpuBuffer* getGpuBuffer() const {
        return gpuBuffer.get();
    }

    std::unique_ptr<GpuBuffer> releaseBuffer() {
        return std::move(gpuBuffer);
    }
};