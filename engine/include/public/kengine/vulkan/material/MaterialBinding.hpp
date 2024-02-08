#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

#include <memory>
#include <future>

class MaterialBindingConfig;
class GpuBuffer;
class CachedGpuBuffer;

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
    CachedGpuBuffer& gpuBuffer;

public:
    BufferBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, CachedGpuBuffer& gpuBuffer)
        : MaterialBinding(bindingConfig), gpuBuffer(gpuBuffer) {}

    CachedGpuBuffer& getGpuBuffer() {
        return gpuBuffer;
    }
};

class ImageBinding : public MaterialBinding {
private:
    Texture2d& texture;

public:
    ImageBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, Texture2d& texture)
        : MaterialBinding(bindingConfig), texture(texture) {}

    Texture2d& getTexture() {
        return texture;
    }
};
