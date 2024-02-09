#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>

int MaterialBinding::getDescriptorSetIndex() {
    return bindingConfig->getDescriptorSetIndex();
}

int MaterialBinding::getBindingIndex() {
    return bindingConfig->getBindingIndex();
}