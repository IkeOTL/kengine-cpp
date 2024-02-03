#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>

#include <future>
#include <kengine/vulkan/texture/AsyncTextureCache.hpp>

class MaterialBindingConfig {
private:
    unsigned int descriptorSetIndex;
    unsigned int bindingIndex;
public:
    MaterialBindingConfig(int descriptorSetIndex, int bindingIndex) :
        descriptorSetIndex(descriptorSetIndex), bindingIndex(bindingIndex) {}

    int getDescriptorSetIndex() {
        return descriptorSetIndex;
    }

    int getBindingIndex() {
        return bindingIndex;
    }

    virtual std::future<MaterialBinding> getBinding(AsyncTextureCache textureCache, GpuBufferCache bufferCache) = 0;

    virtual int hash() = 0;

    int hashCode();

    // hash + equal
};