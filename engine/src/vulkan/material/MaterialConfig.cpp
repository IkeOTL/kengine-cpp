#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/util/MapUtils.hpp>

namespace ke {
    void MaterialConfig::addImageBinding(uint32_t descriptorSetIndex, uint32_t bindingIndex, const TextureConfig& config) {
        auto ptr = std::make_shared<ImageBindingConfig>(descriptorSetIndex, bindingIndex, std::make_shared<TextureConfig>(config));
        bindingConfigs[std::make_pair(descriptorSetIndex, bindingIndex)] = ptr;
    }

    void MaterialConfig::addBufferBinding(uint32_t descriptorSetIndex, uint32_t bindingIndex, GpuBufferId bufferId) {
        auto ptr = std::make_shared<BufferBindingConfig>(descriptorSetIndex, bindingIndex, bufferId);
        bindingConfigs[std::make_pair(descriptorSetIndex, bindingIndex)] = ptr;
    }

    void MaterialConfig::addImageArrayBinding(uint32_t descriptorSetIndex, uint32_t bindingIndex, const std::span<TextureConfig> configs) {
        std::vector<std::shared_ptr<TextureConfig>> outConfigs(configs.size());
        for (auto i = 0; i < configs.size(); i++)
            outConfigs[i] = std::make_shared<TextureConfig>(configs[i]);

        auto ptr = std::make_shared<ImageArrayBindingConfig>(descriptorSetIndex, bindingIndex, std::move(outConfigs));
        bindingConfigs[std::make_pair(descriptorSetIndex, bindingIndex)] = ptr;
    }

    void MaterialConfig::addSkeleton(int skeletonBufferId) {
    }

    size_t MaterialConfig::hashCode() const noexcept {
        ZoneScoped;

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
} // namespace ke