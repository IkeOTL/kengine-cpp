#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>

namespace DescriptorSet {
    struct DescriptorSetLayoutBindingConfig {
    public:
        const size_t bindingIndex;
        const size_t descriptorCount;
        const VkDescriptorType descriptorType;
        const VkShaderStageFlags stageFlags;

        size_t hashCode() const;
        bool operator==(const DescriptorSetLayoutBindingConfig& other) const;
    };

    class DescriptorSetLayoutConfig {
    public:
        const std::vector<DescriptorSetLayoutBindingConfig> bindings;

        DescriptorSetLayoutConfig(std::initializer_list<DescriptorSetLayoutBindingConfig> b)
            : bindings(b) { }

        const DescriptorSetLayoutBindingConfig& getBinding(size_t idx) const;

        size_t hashCode() const;
        bool operator==(const DescriptorSetLayoutConfig& other) const;
    };

    struct DescriptorSetLayoutConfigHasher {
        size_t operator()(const DescriptorSet::DescriptorSetLayoutConfig& obj) const {
            return obj.hashCode();
        }
    };

    class DescriptorSetLayoutCache {
    private:
        std::unordered_map<DescriptorSetLayoutConfig, VkDescriptorSetLayout, DescriptorSetLayoutConfigHasher> descriptorSetLayouts;
        VulkanContext& vulkanCxt;

    public:
        DescriptorSetLayoutCache(VulkanContext& vulkanCxt)
            : vulkanCxt(vulkanCxt) {}

        VkDescriptorSetLayout getLayout(const DescriptorSetLayoutConfig& config);
    };
}