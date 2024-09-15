#include <kengine/vulkan/SamplerCache.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <bit>

VkSampler SamplerCache::getSampler(SamplerConfig& config) {
    {
        std::shared_lock<std::shared_mutex> lock(this->lock);

        auto it = cache.find(config);
        if (it != cache.end())
            return it->second;
    }

    {
        std::unique_lock<std::shared_mutex> lock(this->lock);

        // double check
        auto it = cache.find(config);
        if (it != cache.end())
            return it->second;

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = config.magFilter;
        samplerInfo.minFilter = config.minFilter;
        samplerInfo.mipmapMode = config.mipmapMode;
        samplerInfo.addressModeU = config.addressModeU;
        samplerInfo.addressModeV = config.addressModeV;
        samplerInfo.addressModeW = config.addressModeW;
        samplerInfo.compareOp = config.compareOp;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = config.maxLod;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.borderColor = config.borderColor;
        samplerInfo.maxAnisotropy = 1.0f;

        // Note: If using anisotropic filtering, should also set samplerInfo.anisotropyEnable = VK_TRUE?

        VkSampler sampler;
        VKCHECK(vkCreateSampler(vkCtx.getVkDevice(), &samplerInfo, nullptr, &sampler),
            "Failed to create sampler.");

        cache[config] = sampler;

        return sampler;
    }
}

size_t SamplerConfig::hashCode() const noexcept {
    std::size_t hash = 5;
    hash = 37 * hash + static_cast<std::size_t>(mipmapMode);
    hash = 37 * hash + static_cast<std::size_t>(minFilter);
    hash = 37 * hash + static_cast<std::size_t>(magFilter);
    hash = 37 * hash + static_cast<std::size_t>(addressModeU);
    hash = 37 * hash + static_cast<std::size_t>(addressModeV);
    hash = 37 * hash + static_cast<std::size_t>(addressModeW);
    hash = 37 * hash + static_cast<std::size_t>(compareOp);
    hash = 37 * hash + static_cast<std::size_t>(borderColor);
    hash = 37 * hash + static_cast<std::size_t>(std::bit_cast<int>(minLod));
    hash = 37 * hash + static_cast<std::size_t>(std::bit_cast<int>(maxLod));
    hash = 37 * hash + static_cast<std::size_t>(std::bit_cast<int>(mipLodBias));
    hash = 37 * hash + static_cast<std::size_t>(std::bit_cast<int>(maxAnisotropy));
    return hash;
}

bool SamplerConfig::operator==(const SamplerConfig& other) const {
    if (this == &other)
        return true;

    return mipmapMode == other.mipmapMode &&
        minFilter == other.minFilter &&
        magFilter == other.magFilter &&
        addressModeU == other.addressModeU &&
        addressModeV == other.addressModeV &&
        addressModeW == other.addressModeW &&
        compareOp == other.compareOp &&
        borderColor == other.borderColor &&
        std::bit_cast<int>(minLod) == std::bit_cast<int>(other.minLod) &&
        std::bit_cast<int>(maxLod) == std::bit_cast<int>(other.maxLod) &&
        std::bit_cast<int>(mipLodBias) == std::bit_cast<int>(other.mipLodBias) &&
        std::bit_cast<int>(maxAnisotropy) == std::bit_cast<int>(other.maxAnisotropy);
}

bool SamplerConfig::operator!=(const SamplerConfig& other) const {
    return !(*this == other);
}