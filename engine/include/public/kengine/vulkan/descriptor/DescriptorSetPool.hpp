#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>
#include "DescriptorSetLayout.hpp"

namespace DescriptorSet {
    struct GlobalDescriptorSetHasher {
        size_t operator()(const std::pair<std::string, DescriptorSetLayoutConfig>& obj) const {
            size_t hash = 3;
            hash = 37 * hash + std::hash<std::string>{}(obj.first);
            hash = 37 * hash + obj.second.hashCode();
            return hash;
        }
    };

    class DescriptorSetPool {
    private:
        const static size_t POOL_SIZE = 1000;

        VkDevice vkDevice;
        DescriptorSetLayoutCache& layoutCache;
        VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

        std::unordered_map<DescriptorSetLayoutConfig, std::vector<VkDescriptorSet>, DescriptorSetLayoutConfigHasher> availableDescSets;
        std::unordered_map<DescriptorSetLayoutConfig, std::vector<VkDescriptorSet>, DescriptorSetLayoutConfigHasher> unavailableDescSets;
        std::unordered_map<std::pair<std::string, DescriptorSetLayoutConfig>, VkDescriptorSet, GlobalDescriptorSetHasher> globalDescSets;

        unsigned int createdCount = 0;

        VkDescriptorSet createDescriptorSet(DescriptorSetLayoutConfig config);

    public:
        DescriptorSetPool(VkDevice vkDevice, DescriptorSetLayoutCache& layoutCache)
            : vkDevice(vkDevice), layoutCache(layoutCache) {}

        void init();

        VkDescriptorSet getGlobalDescriptorSet(std::string key, DescriptorSetLayoutConfig config);
        VkDescriptorSet leaseDescriptorSet(DescriptorSetLayoutConfig config);
        void flip();
    };
}