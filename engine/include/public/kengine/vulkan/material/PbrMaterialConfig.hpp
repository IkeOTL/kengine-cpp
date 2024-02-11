#pragma once
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <glm/vec4.hpp>

#include <future>

class VulkanContext;

class MaterialsBuffer {
public:
    static const int MAX_MATERIALS = 1024;

    static int alignedFrameSize(VulkanContext& vkCxt) {
        return (int)vkCxt.alignSsboFrame(size());
    }

    static int size() {
        return MAX_MATERIALS * singleMaterialSize();
    }

    static int singleMaterialSize() {
        return 4 * sizeof(float) // albedoFactor
            + 4 * sizeof(float) // emissiveFactor
            + 1 * sizeof(int32_t) // albedoTexture bool
            + 1 * sizeof(int32_t) // metaiilicRoughnessTexture bool
            + 1 * sizeof(int32_t) // normalTexture bool
            + 1 * sizeof(int32_t) // occusionTexture bool
            + 1 * sizeof(int32_t) // emissiceTexture bool     
            + 1 * sizeof(float) // metallicFactor
            + 1 * sizeof(float) // roughnessFactor
            + 1 * sizeof(float) // padding
            ;
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
    PbrMaterialConfig(std::type_index type) :
        MaterialConfig(type) {}

    void upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) override;
    size_t hash() const noexcept override;

    PbrMaterialConfig& addAlbedoTexture(TextureConfig* config);
    PbrMaterialConfig& addNormalsTexture(TextureConfig* config);
    PbrMaterialConfig& addMetallicRoughnessTexture(TextureConfig* config);
    PbrMaterialConfig& addAmbientOcclusionTexture(TextureConfig* config);
    PbrMaterialConfig& addEmissiveTexture(TextureConfig* config);
};
