#pragma once
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <glm/vec4.hpp>

#include <future>

class VulkanContext;

struct PbrMaterialData {
    const glm::vec4 albedoFactor;
    const glm::vec4 emissiveFactor;
    const int32_t albedoTextureSet;
    const int32_t metallicRoughnessTextureSet;
    const int32_t normalTextureSet;
    const int32_t occlusionTextureSet;
    const int32_t emissiveTextureSet;
    const float metallicFactor;
    const float roughnessFactor;
    const float padding = 0;
};

class MaterialsBuffer {
public:
    static const int MAX_MATERIALS = 1024;

    static int alignedFrameSize(VulkanContext& vkCxt) {
        return (int)vkCxt.alignSsboFrame(size());
    }

    static int size() {
        return MAX_MATERIALS * sizeof(PbrMaterialData);
    }
};

class PbrMaterialConfig : public MaterialConfig {
private:
    glm::vec4 albedoFactor = glm::vec4(1.0f);
    glm::vec4 emissiveFactor = glm::vec4(0);

    int32_t albedoTextureSet = -1;
    int32_t metallicRoughnessTextureSet = -1;
    int32_t normalTextureSet = -1;
    int32_t occlusionTextureSet = -1;
    int32_t emissiveTextureSet = -1;
    float metallicFactor = 0;
    float roughnessFactor = 0;

public:
    inline static const TextureConfig EMPTY_CONFIG = TextureConfig("empty.png");

    PbrMaterialConfig(std::type_index type) :
        MaterialConfig(type) {}

    void upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) override;
    size_t hash() const noexcept override;
    
    void addSkeleton(int skeletonBufferId) override;
    PbrMaterialConfig& addAlbedoTexture(TextureConfig* config);
    PbrMaterialConfig& addNormalsTexture(TextureConfig* config);
    PbrMaterialConfig& addMetallicRoughnessTexture(TextureConfig* config);
    PbrMaterialConfig& addAmbientOcclusionTexture(TextureConfig* config);
    PbrMaterialConfig& addEmissiveTexture(TextureConfig* config);
};
