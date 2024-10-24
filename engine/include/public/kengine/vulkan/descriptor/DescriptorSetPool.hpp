#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanObject.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <vector>
#include <unordered_map>
#include <functional>

namespace ke {
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
        std::unique_ptr<ke::VulkanDescriptorPool> pool = nullptr;

        std::unordered_map<DescriptorSetLayoutConfig, std::vector<VkDescriptorSet>, DescriptorSetLayoutConfigHasher> availableDescSets;
        std::unordered_map<DescriptorSetLayoutConfig, std::vector<VkDescriptorSet>, DescriptorSetLayoutConfigHasher> unavailableDescSets;
        std::unordered_map<std::pair<std::string, DescriptorSetLayoutConfig>, VkDescriptorSet, GlobalDescriptorSetHasher> globalDescSets;

        unsigned int createdCount = 0;

        VkDescriptorSet createDescriptorSet(const DescriptorSetLayoutConfig& config);

    public:
        DescriptorSetPool(VkDevice vkDevice, DescriptorSetLayoutCache& layoutCache)
            : vkDevice(vkDevice), layoutCache(layoutCache) {}

        void init();

        VkDescriptorSet getGlobalDescriptorSet(std::string key, const DescriptorSetLayoutConfig& config);
        VkDescriptorSet leaseDescriptorSet(const DescriptorSetLayoutConfig& config);
        void flip();

        bool operator==(const DescriptorSetPool& other) const {
            if (!pool || !other.pool)
                return false;

            return pool->handle == other.pool->handle;
        }
    };
} // namespace ke