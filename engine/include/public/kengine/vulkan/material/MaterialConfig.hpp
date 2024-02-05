#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <unordered_map>
#include <mutex>
#include <typeinfo>
#include <typeindex>
#include <utility>

class MaterialConfig {
private:
    std::type_index pipelineTypeIndex;

    bool _hasShadow = false;
    bool _hasSkeleton = false;
    int32_t skeletonBufferId = -1;

    std::unordered_map<std::tuple<int, int>, std::shared_ptr<MaterialBindingConfig>> bindingConfigs;

public:
    MaterialConfig(std::type_index type) : pipelineTypeIndex(type) {}

    const std::type_index& getPipeline() const {
        return pipelineTypeIndex;
    }

    const std::unordered_map<std::tuple<int, int>, std::shared_ptr<MaterialBindingConfig>>& getBindingConfigs() const {
        return bindingConfigs;
    }

    template <typename T>
    T& addBinding(int descriptorSetIndex, int bindingIndex) {
        static_assert(std::is_base_of<MaterialBindingConfig, T>::value, "T must be a subclass of MaterialBindingConfig");

        auto key = std::tuple<int, int>(descriptorSetIndex, bindingIndex);
        auto ptr = std::make_shared<T>(descriptorSetIndex, bindingIndex);
        bindingConfigs[key] = ptr;

        return *(ptr->get());
    }

    virtual void addSkeleton(int skeletonBufferId);

    bool hasShadow() const {
        return _hasShadow;
    }

    MaterialConfig& setHasShadow(bool hasShadow) {
        this->_hasShadow = hasShadow;
        return *this;
    }

    bool hasSkeleton() const {
        return _hasSkeleton;
    }

    MaterialConfig& setHasSkeleton(bool hasSkeleton) {
        this->_hasSkeleton = hasSkeleton;
        return *this;
    }

    // hash + equal
};