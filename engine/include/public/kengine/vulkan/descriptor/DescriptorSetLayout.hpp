#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/Hashable.hpp>
#include <vector>
#include <unordered_map>
#include <functional>

namespace ke {
    class VulkanContext;

    struct DescriptorSetLayoutBindingConfig : Hashable {
    public:
        const uint32_t bindingIndex;
        const uint32_t descriptorCount;
        const VkDescriptorType descriptorType;
        const VkShaderStageFlags stageFlags;

        DescriptorSetLayoutBindingConfig(uint32_t bindingIndex, uint32_t descriptorCount,
            VkDescriptorType descriptorType, VkShaderStageFlags stageFlags)
            : bindingIndex(bindingIndex),
              descriptorCount(descriptorCount),
              descriptorType(descriptorType),
              stageFlags(stageFlags) {}

        size_t hashCode() const noexcept override;
        bool operator==(const DescriptorSetLayoutBindingConfig& other) const;
    };

    class DescriptorSetLayoutConfig : Hashable {
    public:
        const std::vector<DescriptorSetLayoutBindingConfig> bindings;

        DescriptorSetLayoutConfig(std::initializer_list<DescriptorSetLayoutBindingConfig> b)
            : bindings(b) {}

        const DescriptorSetLayoutBindingConfig& getBinding(size_t idx) const;

        size_t hashCode() const noexcept override;
        bool operator==(const DescriptorSetLayoutConfig& other) const;
    };

    struct DescriptorSetLayoutConfigHasher {
        size_t operator()(const DescriptorSetLayoutConfig& obj) const {
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

        ~DescriptorSetLayoutCache();

        VkDescriptorSetLayout getLayout(const DescriptorSetLayoutConfig& config);
    };
} // namespace ke
