#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/util/VecUtils.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>

#include <future>

PbrMaterialConfig::PbrMaterialConfig(int skeletonBufferId) :
    PbrMaterialConfig(typeid(SkinnedOffscreenPbrPipeline)) {
    addSkeleton(skeletonBufferId);
}

PbrMaterialConfig::PbrMaterialConfig() :
    PbrMaterialConfig(typeid(DeferredOffscreenPbrPipeline)) {
}

PbrMaterialConfig::PbrMaterialConfig(std::type_index type) :
    MaterialConfig(type) {
    addAlbedoTexture(nullptr);
    addNormalsTexture(nullptr);
    addMetallicRoughnessTexture(nullptr);
    addAmbientOcclusionTexture(nullptr);
    addEmissiveTexture(nullptr);
}

void PbrMaterialConfig::upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) {
    auto mat = PbrMaterialData{
        albedoFactor,
        emissiveFactor,
        albedoTextureSet,
        metallicRoughnessTextureSet,
        normalTextureSet,
        occlusionTextureSet,
        emissiveTextureSet,
        metallicFactor,
        roughnessFactor
    };

    auto pos = gpuBuffer.getFrameOffset(frameIndex) + materialId * sizeof(PbrMaterialData);
    auto buf = static_cast<unsigned char*>(gpuBuffer.getGpuBuffer().data());
    memcpy(buf + pos, &mat, sizeof(PbrMaterialData));
}

size_t PbrMaterialConfig::hash() const noexcept {
    const size_t prime = 67;
    size_t hash = 3;
    hash = prime * hash + vecutils::hashCode(albedoFactor);
    hash = prime * hash + vecutils::hashCode(emissiveFactor);
    hash = prime * hash + albedoTextureSet;
    hash = prime * hash + metallicRoughnessTextureSet;
    hash = prime * hash + normalTextureSet;
    hash = prime * hash + occlusionTextureSet;
    hash = prime * hash + emissiveTextureSet;
    hash = prime * hash + static_cast<std::size_t>(std::bit_cast<int>(metallicFactor));
    hash = prime * hash + static_cast<std::size_t>(std::bit_cast<int>(roughnessFactor));
    return hash;
}

void PbrMaterialConfig::addSkeleton(int skeletonBufferId) {
    if (skeletonBufferId == -1)
        return;

    setHasSkeleton(true);
    setSkeletonBufferId(skeletonBufferId);

    addBufferBinding(2, 5, skeletonBufferId);
}

PbrMaterialConfig& PbrMaterialConfig::addAlbedoTexture(TextureConfig* config) {
    if (!config) {
        addImageBinding(2, 0, EMPTY_CONFIG);
        albedoTextureSet = -1;
        return *this;
    }

    addImageBinding(2, 0, *config);
    albedoTextureSet = 0;
    albedoFactor = glm::vec4(1.0f);
    return *this;
}

PbrMaterialConfig& PbrMaterialConfig::addNormalsTexture(TextureConfig* config) {
    if (!config) {
        addImageBinding(2, 1, EMPTY_CONFIG);
        normalTextureSet = -1;
        return *this;
    }

    addImageBinding(2, 1, *config);
    normalTextureSet = 0;
    return *this;
}

PbrMaterialConfig& PbrMaterialConfig::addMetallicRoughnessTexture(TextureConfig* config) {
    if (!config) {
        addImageBinding(2, 2, EMPTY_CONFIG);
        metallicRoughnessTextureSet = -1;
        return *this;
    }

    addImageBinding(2, 2, *config);
    metallicRoughnessTextureSet = 0;
    metallicFactor = 1;
    roughnessFactor = 1;
    return *this;
}

PbrMaterialConfig& PbrMaterialConfig::addAmbientOcclusionTexture(TextureConfig* config) {
    if (!config) {
        addImageBinding(2, 3, EMPTY_CONFIG);
        occlusionTextureSet = -1;
        return *this;
    }

    addImageBinding(2, 3, *config);
    occlusionTextureSet = 0;
    return *this;
}

PbrMaterialConfig& PbrMaterialConfig::addEmissiveTexture(TextureConfig* config) {
    if (!config) {
        addImageBinding(2, 4, EMPTY_CONFIG);
        emissiveTextureSet = -1;
        return *this;
    }

    addImageBinding(2, 4, *config);
    emissiveTextureSet = 0;
    emissiveFactor = glm::vec4(1.0f);
    return *this;
}

PbrMaterialConfig& PbrMaterialConfig::setMetallicFactor(float f) {
    metallicFactor = f;
    return *this;
}

PbrMaterialConfig& PbrMaterialConfig::setRoughnessFactor(float f) {
    roughnessFactor = f;
    return *this;
}