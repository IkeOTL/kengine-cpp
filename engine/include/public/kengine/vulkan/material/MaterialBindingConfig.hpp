#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/Hashable.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

#include <future>

class MaterialBinding;

class MaterialBindingConfig : public std::enable_shared_from_this<MaterialBindingConfig>, public Hashable {
private:
    uint32_t descriptorSetIndex;
    uint32_t bindingIndex;

public:
    MaterialBindingConfig(uint32_t descriptorSetIndex, uint32_t bindingIndex) :
        descriptorSetIndex(descriptorSetIndex), bindingIndex(bindingIndex) {}

    virtual ~MaterialBindingConfig() = default;

    uint32_t getDescriptorSetIndex() {
        return descriptorSetIndex;
    }

    uint32_t getBindingIndex() {
        return bindingIndex;
    }

    virtual std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) = 0;

    virtual size_t hash() const = 0;

    size_t hashCode() const noexcept override;
};

class BufferBindingConfig : public MaterialBindingConfig {
private:
    GpuBufferId bufferCacheKey;

public:
    BufferBindingConfig(uint32_t descriptorSetIndex, uint32_t bindingIndex, GpuBufferId bufferCacheKey)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), bufferCacheKey(bufferCacheKey) {}

    std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    size_t hash() const override;
};

class ImageBindingConfig : public MaterialBindingConfig {
private:
    // use shared pointers? ask opinions
    std::shared_ptr<TextureConfig> textureConfig;

public:
    ImageBindingConfig(uint32_t descriptorSetIndex, uint32_t bindingIndex, std::shared_ptr<TextureConfig> textureConfig)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), textureConfig(textureConfig) {}

    std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    size_t hash() const override;
};

class ImageArrayBindingConfig : public MaterialBindingConfig {
private:
    // use shared pointers? ask opinions
    std::vector<std::shared_ptr<TextureConfig>> textureConfigs;

public:
    ImageArrayBindingConfig(uint32_t descriptorSetIndex, uint32_t bindingIndex, std::vector<std::shared_ptr<TextureConfig>> textureConfigs)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), textureConfigs(textureConfigs) {}

    std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    size_t hash() const override;
};

