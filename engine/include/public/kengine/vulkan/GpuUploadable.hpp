#pragma once
#include <kengine/vulkan/GpuBuffer.hpp>
#include <memory>

class VulkanContext;

class GpuUploadable {
private:
    std::unique_ptr<GpuBuffer> gpuBuffer;

public:
    virtual void upload(VulkanContext& vkCxt, void* data) = 0;
    virtual VkDeviceSize size() = 0;

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