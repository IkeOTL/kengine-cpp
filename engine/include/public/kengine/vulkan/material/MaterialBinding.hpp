#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

#include <memory>
#include <future>

class MaterialBindingConfig;
class GpuBuffer;

class MaterialBinding {
private:
    const std::shared_ptr<MaterialBindingConfig> bindingConfig;

public:
    MaterialBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig)
        : bindingConfig(bindingConfig) {}

    int getDescriptorSetIndex();
    int getBindingIndex();
};

class BufferBinding : public MaterialBinding {
private:
    GpuBuffer& gpuBuffer;

public:
    BufferBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, GpuBuffer& gpuBuffer)
        : MaterialBinding(bindingConfig), gpuBuffer(gpuBuffer) {}

    GpuBuffer& getGpuBuffer() {
        return gpuBuffer;
    }
};

class ImageBinding : public MaterialBinding {
private:

public:
    ImageBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig)
        : MaterialBinding(bindingConfig) {}

};
