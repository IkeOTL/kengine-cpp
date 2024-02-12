#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/Hashable.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <unordered_map>
#include <mutex>
#include <typeinfo>
#include <typeindex>
#include <utility>
#include <functional>

class MaterialConfig : Hashable {
private:
    struct IntPairHash {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1, T2>& pair) const {
            const std::size_t prime = 31;
            std::size_t hash1 = std::hash<T1>{}(pair.first);
            std::size_t hash2 = std::hash<T2>{}(pair.second);
            return hash1 * prime + hash2;
        }
    };

    std::type_index pipelineTypeIndex;

    bool _hasShadow = false;
    bool _hasSkeleton = false;
    int32_t skeletonBufferId = -1;

    std::unordered_map<std::pair<int, int>, std::shared_ptr<MaterialBindingConfig>, IntPairHash> bindingConfigs;

public:
    MaterialConfig(std::type_index type) : pipelineTypeIndex(type) {}

    const std::type_index& getPipeline() const {
        return pipelineTypeIndex;
    }

    const std::unordered_map<std::pair<int, int>, std::shared_ptr<MaterialBindingConfig>, IntPairHash>& getBindingConfigs() const {
        return bindingConfigs;
    }

    void addImageBinding(unsigned int descriptorSetIndex, unsigned int bindingIndex, TextureConfig config);

    void addBufferBinding(unsigned int descriptorSetIndex, unsigned int bindingIndex, int bufferId);

    virtual void addSkeleton(int skeletonBufferId) = 0;

    bool hasShadow() const {
        return _hasShadow;
    }

    MaterialConfig& setHasShadow(bool hasShadow) {
        this->_hasShadow = hasShadow;
        return *this;
    }

    bool hasSkeleton() const {
        return _hasSkeleton;
    }

    MaterialConfig& setHasSkeleton(bool hasSkeleton) {
        this->_hasSkeleton = hasSkeleton;
        return *this;
    }
    virtual void upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) = 0;

    virtual size_t hash() const noexcept = 0;

    size_t hashCode() const noexcept override;

    bool operator==(const MaterialConfig& other) const;
    bool operator!=(const MaterialConfig& other) const;

protected:
    MaterialConfig& setSkeletonBufferId(int skeletonBufferId) {
        this->skeletonBufferId = skeletonBufferId;
        return *this;
    }
};

namespace std {
    template<>
    struct hash<MaterialConfig> {
        size_t operator()(const MaterialConfig& p) const noexcept {
            return p.hashCode();
        }
    };
}