#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

#include <future>

class MaterialBinding;

class MaterialBindingConfig {
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

    virtual std::future<std::unique_ptr<MaterialBinding>> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) = 0;

    virtual int hash() = 0;

    int hashCode();
};

class BufferBindingConfig : MaterialBindingConfig {
private:
    int bufferCacheKey;

public:
    BufferBindingConfig(unsigned int descriptorSetIndex, unsigned int bindingIndex, int bufferCacheKey)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), bufferCacheKey(bufferCacheKey) {}

    std::future<std::unique_ptr<MaterialBinding>> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    int hash() override;
};

class ImageBindingConfig : MaterialBindingConfig {
private:
    // use shared pointers? as opinions
    TextureConfig textureConfig;

public:
    ImageBindingConfig(unsigned int descriptorSetIndex, unsigned int bindingIndex, TextureConfig textureConfig)
        : MaterialBindingConfig(descriptorSetIndex, bindingIndex), textureConfig(textureConfig) {}

    std::future<std::unique_ptr<MaterialBinding>> getBinding(AsyncTextureCache& textureCache, GpuBufferCache& bufferCache) override;

    int hash() override;
};

