#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>

using namespace DescriptorSet;

DescriptorSetPool& DescriptorSetAllocator::getPool()
{
    if (!availablePools.empty()) 
        return *availablePools.back();

    //gett from back
    return DescriptorSetPool();
}

void DescriptorSetAllocator::init()
{
    globalPool.init();
}

VkDescriptorSet DescriptorSetAllocator::leaseDescriptorSet(DescriptorSetLayoutConfig& config)
{
    auto& pool = getPool();

    auto set = pool.leaseDescriptorSet(config);

    if (set == VK_NULL_HANDLE) {
        unavailablePools.push_back(std::move(availablePools.back()));
        availablePools.pop_back();

        pool = getPool();
    }
}

void DescriptorSetAllocator::reset() {
    // move from unavail to avail
    while (!unavailablePools.empty()) {
        availablePools.push_back(std::move(unavailablePools.back()));
        unavailablePools.pop_back();
    }

    // reset all pools
    for (auto& p : availablePools)
        p->flip();
}
