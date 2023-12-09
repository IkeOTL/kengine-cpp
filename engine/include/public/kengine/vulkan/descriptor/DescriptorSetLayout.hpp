#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace DescriptorSet {
    struct DescriptorSetLayoutBindingConfig {
    public:
        const size_t bindingIndex;
        const size_t descriptorCount;
        const int descriptorType;
        const int stageFlags;

        size_t hashCode() const {
            size_t hash = 7;
            hash = 41 * hash + bindingIndex;
            hash = 41 * hash + descriptorCount;
            hash = 41 * hash + descriptorType;
            hash = 41 * hash + stageFlags;
            return hash;
        }
    };

    struct DescriptorSetLayoutConfig {
    public:
        const std::vector<DescriptorSetLayoutBindingConfig> bindings;

        DescriptorSetLayoutConfig(std::initializer_list<DescriptorSetLayoutBindingConfig> b)
            : bindings(b) { }

        const DescriptorSetLayoutBindingConfig& getBinding(size_t idx) const {
            return bindings[idx];
        }

        size_t hashCode() const {
            size_t hash = 7;
            for (const auto& item : bindings)
                hash = 53 * hash + item.hashCode();
            return hash;
        }
    };

    class DescriptorSetLayoutCache {
    private:

    public:

    };
}

namespace std {
    using namespace DescriptorSet;

    template <>
    struct hash<DescriptorSetLayoutConfig> {
        size_t operator()(const DescriptorSetLayoutConfig& obj) const {
            return obj.hashCode();
        }
    };

    template <>
    struct hash<DescriptorSetLayoutBindingConfig> {
        size_t operator()(const DescriptorSetLayoutBindingConfig& obj) const {
            return obj.hashCode();
        }
    };
}