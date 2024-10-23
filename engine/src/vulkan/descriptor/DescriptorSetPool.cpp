#include <kengine/vulkan/descriptor/DescriptorSetPool.hpp>

void DescriptorSetPool::init() {
    std::vector<VkDescriptorPoolSize> typeCounts(6);
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = static_cast<uint32_t>(POOL_SIZE * 0.1);

    typeCounts[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    typeCounts[1].descriptorCount = static_cast<uint32_t>(POOL_SIZE * 0.1);

    typeCounts[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    typeCounts[2].descriptorCount = static_cast<uint32_t>(POOL_SIZE * 0.2);

    typeCounts[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    typeCounts[3].descriptorCount = static_cast<uint32_t>(POOL_SIZE * 0.1);

    typeCounts[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    typeCounts[4].descriptorCount = static_cast<uint32_t>(POOL_SIZE * 0.5);

    typeCounts[5].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    typeCounts[5].descriptorCount = static_cast<uint32_t>(POOL_SIZE * 0.1);

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(typeCounts.size());
    descriptorPoolInfo.pPoolSizes = typeCounts.data();
    descriptorPoolInfo.maxSets = POOL_SIZE;

    VkDescriptorPool descriptorPool;
    VKCHECK(vkCreateDescriptorPool(vkDevice, &descriptorPoolInfo, nullptr, &descriptorPool),
        "Failed to create descriptor pool");

    pool = std::make_unique<ke::VulkanDescriptorPool>(vkDevice, descriptorPool);
}

VkDescriptorSet DescriptorSetPool::getGlobalDescriptorSet(std::string key, const DescriptorSetLayoutConfig& config) {
    auto woah = std::pair<std::string, DescriptorSetLayoutConfig>(key, config);

    auto it = globalDescSets.find(woah);
    if (it != globalDescSets.end())
        return it->second;

    auto set = createDescriptorSet(config);
    globalDescSets[woah] = set;

    return set;
}

VkDescriptorSet DescriptorSetPool::leaseDescriptorSet(const DescriptorSetLayoutConfig& config) {
    auto& descSets = availableDescSets[config];

    // have some pooled sets that are available?
    if (!descSets.empty()) {
        // grab available set and remove from available list
        auto set = descSets.front();
        descSets.erase(descSets.begin());

        // add to unavailable list
        unavailableDescSets[config].push_back(set);

        return set;
    }

    // none available, try to create new one
    auto set = createDescriptorSet(config);

    // pool depleted?
    if (set == VK_NULL_HANDLE)
        return VK_NULL_HANDLE;

    unavailableDescSets[config].push_back(set);

    return set;
}

VkDescriptorSet DescriptorSetPool::createDescriptorSet(const DescriptorSetLayoutConfig& config) {
    if (createdCount == POOL_SIZE)
        return VK_NULL_HANDLE;

    auto layout = layoutCache.getLayout(config);

    VkDescriptorSetAllocateInfo dsInfo{};
    dsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsInfo.descriptorPool = pool->handle;
    dsInfo.descriptorSetCount = 1;
    dsInfo.pSetLayouts = &layout;

    VkDescriptorSet descriptorSet;
    VKCHECK(vkAllocateDescriptorSets(vkDevice, &dsInfo, &descriptorSet),
        "Failed to allocate descriptor sets.");
    createdCount++;

    return descriptorSet;
}

void DescriptorSetPool::flip() {
    for (auto& unavailSets : unavailableDescSets) {
        auto& availSets = availableDescSets[unavailSets.first];
        for (auto& u : unavailSets.second)
            availSets.push_back(u);
        // vkResetDescriptorPool(engine.getDevice(), poolHandle, 0);
        // createdCount = 0;
        unavailSets.second.clear();
    }
}
