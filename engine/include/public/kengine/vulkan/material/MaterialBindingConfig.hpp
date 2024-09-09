#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/Hashable.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

#include <future>

class MaterialBinding;

class MaterialBindingConfig : public std::enable_shared_from_this<MaterialBindingConfig>, public Hashable {
private:
    unsigned int descriptorSetIndex;
    unsigned int bindingIndex;

public:
    MaterialBindingConfig(unsigned int descriptorSetIndex, unsigned int bindingIndex) :
        descriptorSetIndex(descriptorSetIndex), bindingIndex(bindingIndex) {}

    virtual ~MaterialBindingConfig() = default;

    unsigned int getDescriptorSetIndex() {
        return descriptorSetIndex;
    }

    unsigned int getBindingIndex() {
        return bindingIndex;
    }

    virtual std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) = 0;

    virtual int hash() const = 0;

    size_t hashCode() const noexcept override;
};

class BufferBindingConfig : public MaterialBindingConfig {
private:
    int bufferCacheKey;

public:
    BufferBindingConfig(unsigned int descriptorSetIndex, unsigned int bindingIndex, int bufferCacheKey)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), bufferCacheKey(bufferCacheKey) {}

    std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    int hash() const override;
};

class ImageBindingConfig : public MaterialBindingConfig {
private:
    // use shared pointers? ask opinions
    std::shared_ptr<TextureConfig> textureConfig;

public:
    ImageBindingConfig(unsigned int descriptorSetIndex, unsigned int bindingIndex, std::shared_ptr<TextureConfig> textureConfig)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), textureConfig(textureConfig) {}

    std::unique_ptr<MaterialBinding> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    int hash() const override;
};

