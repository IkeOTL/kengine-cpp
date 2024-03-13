#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/util/MapUtils.hpp>

void MaterialConfig::addImageBinding(unsigned int descriptorSetIndex, unsigned int bindingIndex, TextureConfig config) {
    auto ptr = std::make_shared<ImageBindingConfig>(descriptorSetIndex, bindingIndex, std::make_shared<TextureConfig>(config));
    bindingConfigs[std::make_pair(descriptorSetIndex, bindingIndex)] = ptr;
}

void MaterialConfig::addBufferBinding(unsigned int descriptorSetIndex, unsigned int bindingIndex, int bufferId) {
    auto ptr = std::make_shared<BufferBindingConfig>(descriptorSetIndex, bindingIndex, bufferId);
    bindingConfigs[std::make_pair(descriptorSetIndex, bindingIndex)] = ptr;
}

void MaterialConfig::addSkeleton(int skeletonBufferId)
{
}

size_t MaterialConfig::hashCode() const noexcept {
    size_t hash = 5;
    size_t prime = 53;
    hash = prime * hash + pipelineTypeIndex.hash_code();
    hash = prime * hash + MapHasher::hash(this->bindingConfigs); // cache somehow
    hash = prime * hash + (_hasShadow ? 1 : 0);
    hash = prime * hash + (_hasSkeleton ? 1 : 0);
    hash = prime * hash + skeletonBufferId;
    hash = prime * hash + this->hash();
    return hash;
}

bool MaterialConfig::operator==(const MaterialConfig& other) const {
    if (this == &other)
        return true;

    if (!(pipelineTypeIndex == other.pipelineTypeIndex))
        return false;

    if (!(bindingConfigs == other.bindingConfigs))
        return false;

    return this->hashCode() == other.hashCode();
}

bool MaterialConfig::operator!=(const MaterialConfig& other) const {
    return !(*this == other);
}