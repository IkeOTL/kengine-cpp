#pragma once
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <glm/vec4.hpp>

#include <future>
#include <span>
#include <kengine/vulkan/pipelines/TerrainDeferredOffscreenPbrPipeline.hpp>

namespace ke {
    class VulkanContext;

    class TerrainPbrMaterialConfig : public MaterialConfig {
    private:
        glm::vec4 albedoFactor = glm::vec4(1.0f);
        glm::vec4 emissiveFactor = glm::vec4(0);
        float metallicFactor = 0;
        float roughnessFactor = 0;
        uint32_t textureSetFlags = 0;

    public:
        TerrainPbrMaterialConfig()
            : MaterialConfig(typeid(TerrainDeferredOffscreenPbrPipeline)) {}

        inline static std::shared_ptr<TerrainPbrMaterialConfig> create() {
            return std::make_shared<TerrainPbrMaterialConfig>();
        }

        void upload(VulkanContext& vkCxt, const CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) override;
        size_t hash() const noexcept override;

        void addSkeleton(int skeletonBufferId) override {
            // noop
        }

        TerrainPbrMaterialConfig& addAlbedoTextures(const std::initializer_list<TextureConfig> config);
        TerrainPbrMaterialConfig& addNormalsTextures(const std::initializer_list<TextureConfig> config);
        TerrainPbrMaterialConfig& addMetallicRoughnessTextures(const std::initializer_list<TextureConfig> config);
        TerrainPbrMaterialConfig& addAmbientOcclusionTextures(const std::initializer_list<TextureConfig> config);
        TerrainPbrMaterialConfig& addEmissiveTextures(const std::initializer_list<TextureConfig> config);
        TerrainPbrMaterialConfig& setMetallicFactor(float metallicFactor);
        TerrainPbrMaterialConfig& setRoughnessFactor(float roughnessFactor);
    };
} // namespace ke
