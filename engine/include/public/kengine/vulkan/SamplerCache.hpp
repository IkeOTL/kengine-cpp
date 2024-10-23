#pragma once
#include <kengine/Hashable.hpp>
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanObject.hpp>
#include <functional>
#include <shared_mutex>
#include <memory>

class VulkanContext;

class SamplerConfig : public Hashable {
public:
    const VkSamplerMipmapMode mipmapMode;
    const VkFilter minFilter;
    const VkFilter magFilter;
    const VkSamplerAddressMode addressModeU;
    const VkSamplerAddressMode addressModeV;
    const VkSamplerAddressMode addressModeW;
    const VkCompareOp compareOp;
    const VkBorderColor borderColor;
    const float minLod, maxLod, mipLodBias, maxAnisotropy;

    SamplerConfig(VkSamplerMipmapMode mipmapMode, VkFilter minFilter, VkFilter magFilter,
        VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW,
        VkCompareOp compareOp, VkBorderColor borderColor,
        float minLod, float maxLod,
        float mipLodBias, float maxAnisotropy) :
        mipmapMode(mipmapMode), minFilter(minFilter), magFilter(magFilter),
        addressModeU(addressModeU), addressModeV(addressModeV), addressModeW(addressModeW),
        compareOp(compareOp), borderColor(borderColor),
        minLod(minLod), maxLod(maxLod),
        mipLodBias(mipLodBias), maxAnisotropy(maxAnisotropy) {}

    size_t hashCode() const noexcept override;
    bool operator==(const SamplerConfig& other) const;
    bool operator!=(const SamplerConfig& other) const;
};

namespace std {
    template<>
    struct hash<SamplerConfig> {
        size_t operator()(const SamplerConfig& p) const noexcept {
            return p.hashCode();
        }
    };
}

class SamplerCache {
private:
    VulkanContext& vkCtx;
    std::shared_mutex lock{};
    std::unordered_map<SamplerConfig, std::unique_ptr<ke::VulkanSampler>> cache{};

public:
    SamplerCache(VulkanContext& vkCtx) : vkCtx(vkCtx) {}

    VkSampler getSampler(SamplerConfig& config);
};