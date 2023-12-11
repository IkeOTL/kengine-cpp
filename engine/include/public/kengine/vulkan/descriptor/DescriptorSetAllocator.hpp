#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include "DescriptorSetLayout.hpp"
#include "DescriptorSetPool.hpp"

namespace DescriptorSet {
    class DescriptorSetAllocator {
    private:
        VkDevice vkDevice;
        DescriptorSetLayoutCache& layoutCache;

        DescriptorSetPool globalPool;
        std::vector<std::unique_ptr<DescriptorSetPool>> availablePools;
        std::vector<std::unique_ptr<DescriptorSetPool>> unavailablePools;

        DescriptorSetPool& getPool();
    public:
        DescriptorSetAllocator(VkDevice vkDevice, DescriptorSetLayoutCache& layoutCache)
            : vkDevice(vkDevice), layoutCache(layoutCache), globalPool(vkDevice, layoutCache)
        {}

        void init();
        void reset();
        VkDescriptorSet getGlobalDescriptorSet(DescriptorSetLayoutConfig& config);
        VkDescriptorSet getGlobalDescriptorSet(std::string key, DescriptorSetLayoutConfig& config);
        VkDescriptorSet leaseDescriptorSet(DescriptorSetLayoutConfig& config);
    };
}