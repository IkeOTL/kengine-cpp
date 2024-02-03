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

    bool _hasShadow = false;
    bool _hasSkeleton = false;
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

    virtual void addSkeleton(int skeletonBufferId);

    bool hasShadow() {
        return _hasShadow;
    }

    MaterialConfig setHasShadow(bool hasShadow) {
        this._hasShadow = hasShadow;
        return this;
    }

    bool hasSkeleton() {
        return _hasSkeleton;
    }

    MaterialConfig setHasSkeleton(bool hasSkeleton) {
        this._hasSkeleton = hasSkeleton;
        return this;
    }

    // hash + equal
};