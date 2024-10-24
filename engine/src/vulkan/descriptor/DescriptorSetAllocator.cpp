#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <tracy/Tracy.hpp>

namespace ke {
    void DescriptorSetAllocator::init() {
        globalPool.init();
    }

    VkDescriptorSet DescriptorSetAllocator::getGlobalDescriptorSet(std::string key, const DescriptorSetLayoutConfig& config) {
        std::lock_guard<std::mutex> lock(globalPoolMtx);
        auto set = globalPool.getGlobalDescriptorSet(key, config);

        // global pool exhausted
        if (set == VK_NULL_HANDLE)
            throw std::runtime_error("Global descriptor set pool is exhausted.");

        return set;
    }

    VkDescriptorSet DescriptorSetAllocator::leaseDescriptorSet(const DescriptorSetLayoutConfig& config) {
        std::lock_guard<std::mutex> lock(leasePoolMtx);

        auto set = getPool().leaseDescriptorSet(config);

        // pool exhausted or broken?
        if (set == VK_NULL_HANDLE) {
            unavailablePools.push_back(std::move(availablePools.back()));
            availablePools.pop_back();

            // try next pool
            set = getPool().leaseDescriptorSet(config);
        }

        // if still null then something serious is up
        if (set == VK_NULL_HANDLE)
            throw std::runtime_error("Something is wrong with DescriptorSetAllocator.");

        return set;
    }

    DescriptorSetPool& DescriptorSetAllocator::getPool() {
        if (!availablePools.empty())
            return *availablePools.back();

        auto pool = std::make_unique<DescriptorSetPool>(vkDevice, layoutCache);
        pool->init();
        availablePools.push_back(std::move(pool));
        return *availablePools.back();
    }

    void DescriptorSetAllocator::reset() {
        ZoneScoped;

        // move from unavail to avail
        while (!unavailablePools.empty()) {
            availablePools.push_back(std::move(unavailablePools.back()));
            unavailablePools.pop_back();
        }

        // reset all pools
        for (auto& p : availablePools)
            p->flip();
    }
} // namespace ke
