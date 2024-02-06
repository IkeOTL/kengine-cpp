#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>

int MaterialBinding::getDescriptorSetIndex() {
    return bindingConfig->getDescriptorSetIndex();
}

int MaterialBinding::getBindingIndex() {
    return bindingConfig->getBindingIndex();
}