#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>

using namespace DescriptorSet;

const DescriptorSetLayoutBindingConfig& DescriptorSetLayoutConfig::getBinding(size_t idx) const {
    return bindings[idx];
}

size_t DescriptorSetLayoutBindingConfig::hashCode() const {
    size_t hash = 7;
    hash = 41 * hash + bindingIndex;
    hash = 41 * hash + descriptorCount;
    hash = 41 * hash + descriptorType;
    hash = 41 * hash + stageFlags;
    return hash;
}

bool DescriptorSetLayoutBindingConfig::operator==(const DescriptorSetLayoutBindingConfig& other) const {
    if (this == &other)
        return true;

    if (this->bindingIndex != other.bindingIndex)
        return false;

    if (this->descriptorCount != other.descriptorCount)
        return false;

    if (this->descriptorType != other.descriptorType)
        return false;

    return this->stageFlags == other.stageFlags;
}

size_t DescriptorSetLayoutConfig::hashCode() const {
    size_t hash = 7;
    for (const auto& item : bindings)
        hash = 53 * hash + item.hashCode();
    return hash;
}

bool DescriptorSetLayoutConfig::operator==(const DescriptorSetLayoutConfig& other) const {
    if (this == &other)
        return true;

    return this->bindings == other.bindings;
}

VkDescriptorSetLayout DescriptorSetLayoutCache::getLayout(const DescriptorSetLayoutConfig& config) {
    auto it = descriptorSetLayouts.find(config);

    // found in cache
    if (it != descriptorSetLayouts.end())
        return it->second;

    auto bindingConfigs = &config.bindings;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& bc : config.bindings) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = bc.bindingIndex;
        binding.descriptorCount = bc.descriptorCount;
        binding.descriptorType = bc.descriptorType;
        binding.stageFlags = bc.stageFlags;
        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    VKCHECK(vkCreateDescriptorSetLayout(vulkanCxt.getVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout),
        "Failed to create VkDescriptorSetLayout.");

    descriptorSetLayouts[config] = descriptorSetLayout;

    return descriptorSetLayout;
}