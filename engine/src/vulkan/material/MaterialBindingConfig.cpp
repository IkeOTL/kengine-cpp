#include <kengine/vulkan/material/MaterialBindingConfig.hpp>

int MaterialBindingConfig::hashCode() {
    int hash = 5;
    hash = 73 * hash + descriptorSetIndex;
    hash = 73 * hash + bindingIndex;
    hash = 73 * hash + this->hash();
    return hash;
}