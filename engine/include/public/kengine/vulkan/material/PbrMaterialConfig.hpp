#pragma once
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <glm/vec4.hpp>

#include <future>

namespace ke {
    class VulkanContext;

    enum TextureSetFlag {
        ALBEDO_TEXTURE_SET = 1 << 0,
        METALLIC_ROUGHNESS_SET = 1 << 1,
        NORMAL_TEXTURE_SET = 1 << 2,
        OCCLUSION_TEXTURE_SET = 1 << 3,
        EMISSIVE_TEXTURE_SET = 1 << 4,
    };

    struct PbrMaterialData {
        const glm::vec4 albedoFactor;
        const glm::vec4 emissiveFactor;
        const float metallicFactor;
        const float roughnessFactor;
        const uint32_t textureSetFlags;
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
        float metallicFactor = 0;
        float roughnessFactor = 0;
        uint32_t textureSetFlags = 0;

    public:
        inline static const TextureConfig EMPTY_CONFIG = TextureConfig("img/empty.png");

        PbrMaterialConfig();
        PbrMaterialConfig(std::type_index type);
        PbrMaterialConfig(int skeletonBufferId);

        inline static std::shared_ptr<PbrMaterialConfig> create() {
            return std::make_shared<PbrMaterialConfig>();
        }

        inline static std::shared_ptr<PbrMaterialConfig> create(std::type_index type) {
            return std::make_shared<PbrMaterialConfig>(type);
        }

        inline static std::shared_ptr<PbrMaterialConfig> create(int skeletonBufferId) {
            return std::make_shared<PbrMaterialConfig>(skeletonBufferId);
        }

        void upload(VulkanContext& vkCxt, const CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) override;
        size_t hash() const noexcept override;

        void addSkeleton(int skeletonBufferId) override;
        PbrMaterialConfig& addAlbedoTexture(TextureConfig* config);
        PbrMaterialConfig& addNormalsTexture(TextureConfig* config);
        PbrMaterialConfig& addMetallicRoughnessTexture(TextureConfig* config);
        PbrMaterialConfig& addAmbientOcclusionTexture(TextureConfig* config);
        PbrMaterialConfig& addEmissiveTexture(TextureConfig* config);
        PbrMaterialConfig& setMetallicFactor(float metallicFactor);
        PbrMaterialConfig& setRoughnessFactor(float roughnessFactor);
    };
} // namespace ke
