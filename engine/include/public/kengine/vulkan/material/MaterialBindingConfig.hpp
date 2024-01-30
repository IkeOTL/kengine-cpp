#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

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

    virtual int subHash() = 0;
    
    // hash + equal
};