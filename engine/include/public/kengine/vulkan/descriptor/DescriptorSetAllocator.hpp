#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include "DescriptorSetLayout.hpp"
#include "DescriptorSetPool.hpp"
#include <mutex>

namespace DescriptorSet {
    class DescriptorSetAllocator {
    private:
        VkDevice vkDevice;
        DescriptorSetLayoutCache& layoutCache;

        DescriptorSetPool globalPool;
        std::vector<std::unique_ptr<DescriptorSetPool>> availablePools;
        std::vector<std::unique_ptr<DescriptorSetPool>> unavailablePools;

        std::mutex globalPoolMtx;
        std::mutex leasePoolMtx;

        DescriptorSetPool& getPool();
    public:
        DescriptorSetAllocator(VkDevice vkDevice, DescriptorSetLayoutCache& layoutCache)
            : vkDevice(vkDevice), layoutCache(layoutCache), globalPool(vkDevice, layoutCache)
        {}

        void init();
        void reset();
        VkDescriptorSet getGlobalDescriptorSet(std::string key, const DescriptorSetLayoutConfig& config);
        VkDescriptorSet leaseDescriptorSet(const DescriptorSetLayoutConfig& config);
    };
}