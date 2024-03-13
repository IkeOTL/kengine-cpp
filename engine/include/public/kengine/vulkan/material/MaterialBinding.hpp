#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

#include <memory>
#include <future>

class MaterialBindingConfig;
class GpuBuffer;
class CachedGpuBuffer;
class Texture2d;
class VulkanContext;
class DescriptorSetLayoutConfig;

class MaterialBinding {
private:
    const std::shared_ptr<MaterialBindingConfig> bindingConfig;

public:
    MaterialBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig)
        : bindingConfig(bindingConfig) {}

    int getDescriptorSetIndex();
    int getBindingIndex();

    virtual void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet setWrite, VkDescriptorSet dstSet,
        const DescriptorSetLayoutConfig& layoutConfig, std::vector<uint32_t> offsets) = 0;
};

class BufferBinding : public MaterialBinding {
private:
    CachedGpuBuffer& gpuBuffer;

public:
    BufferBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, CachedGpuBuffer& gpuBuffer)
        : MaterialBinding(bindingConfig), gpuBuffer(gpuBuffer) {}

    const CachedGpuBuffer& getGpuBuffer() const {
        return gpuBuffer;
    }

    void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet setWrite, VkDescriptorSet dstSet,
        const DescriptorSetLayoutConfig& layoutConfig, std::vector<uint32_t> offsets) override;
};

class ImageBinding : public MaterialBinding {
private:
    Texture2d& texture;

public:
    ImageBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, Texture2d& texture)
        : MaterialBinding(bindingConfig), texture(texture) {}

    const Texture2d& getTexture() const {
        return texture;
    }

    void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet setWrite, VkDescriptorSet dstSet,
        const DescriptorSetLayoutConfig& layoutConfig, std::vector<uint32_t> offsets) override;
};
