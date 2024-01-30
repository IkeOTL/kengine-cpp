#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <unordered_map>
#include <mutex>
#include <typeinfo>
#include <typeindex>

template <typename T>
class MaterialConfig {
private:
    std::type_index pipelineTypeIndex;

    bool hasShadow = false;
    bool hasSkeleton = false;
    int32_t skeletonBufferId = -1;

    std::unordered_map<std::pair<int, int>, std::unique_ptr<MaterialBindingConfig>> bindingConfigs;

public:
    MaterialConfig() : pipelineTypeIndex(typeid(T)) {}

    const std::type_index& getPipeline() const {
        return pipelineTypeIndex;
    }

    const std::unordered_map<std::pair<int, int>, MaterialBindingConfig>& getBindingConfigs() const {
        return bindingConfigs;
    }

    void addBinding(std::unique_ptr<MaterialBindingConfig>&& config) {
        bindingConfigs[std::pair(config->)] = std::move(config);
    }

    bool hasShadow() {
        return hasShadow;
    }

    MaterialConfig setHasShadow(boolean hasShadow) {
        this.hasShadow = hasShadow;
        return this;
    }

    bool hasSkeleton() {
        return hasSkeleton;
    }

    MaterialConfig setHasSkeleton(boolean hasSkeleton) {
        this.hasSkeleton = hasSkeleton;
        return this;
    }


    // hash + equal
};