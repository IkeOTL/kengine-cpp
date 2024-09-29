#pragma once
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <glm/vec4.hpp>

#include <future>

class VulkanContext;

class TerrainPbrMaterialConfig : public MaterialConfig {
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
    TerrainPbrMaterialConfig();

    inline static std::shared_ptr<TerrainPbrMaterialConfig> create() {
        return std::make_shared<TerrainPbrMaterialConfig>();
    }

    void upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) override;
    size_t hash() const noexcept override;

    void addSkeleton(int skeletonBufferId) override {
        // noop
    }

    TerrainPbrMaterialConfig& addAlbedoTexture(TextureConfig* config);
    TerrainPbrMaterialConfig& addNormalsTexture(TextureConfig* config);
    TerrainPbrMaterialConfig& addMetallicRoughnessTexture(TextureConfig* config);
    TerrainPbrMaterialConfig& addAmbientOcclusionTexture(TextureConfig* config);
    TerrainPbrMaterialConfig& addEmissiveTexture(TextureConfig* config);
    TerrainPbrMaterialConfig& setMetallicFactor(float metallicFactor);
    TerrainPbrMaterialConfig& setRoughnessFactor(float roughnessFactor);
};
