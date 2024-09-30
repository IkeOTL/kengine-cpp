#include <kengine/vulkan/material/TerrainPbrMaterialConfig.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/util/VecUtils.hpp>

#include <future>

void TerrainPbrMaterialConfig::upload(VulkanContext& vkCxt, const CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) {
    auto mat = PbrMaterialData{
        albedoFactor,
        emissiveFactor,
        metallicFactor,
        roughnessFactor,
        textureSetFlags
    };

    auto pos = gpuBuffer.getFrameOffset(frameIndex) + materialId * sizeof(PbrMaterialData);
    auto buf = static_cast<unsigned char*>(gpuBuffer.getGpuBuffer().data());
    memcpy(buf + pos, &mat, sizeof(PbrMaterialData));
}

size_t TerrainPbrMaterialConfig::hash() const noexcept {
    const size_t prime = 67;
    size_t hash = 3;
    hash = prime * hash + vecutils::hashCode(albedoFactor);
    hash = prime * hash + vecutils::hashCode(emissiveFactor);
    hash = prime * hash + static_cast<std::size_t>(std::bit_cast<int>(metallicFactor));
    hash = prime * hash + static_cast<std::size_t>(std::bit_cast<int>(roughnessFactor));
    hash = prime * hash + textureSetFlags;
    return hash;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addAlbedoTextures(const std::initializer_list<TextureConfig> configs) {
    assert(configs.size() > 0);
    std::vector<TextureConfig> configVec(configs);
    addImageArrayBinding(2, 0, configVec);
    textureSetFlags |= TextureSetFlag::ALBEDO_TEXTURE_SET;
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addNormalsTextures(const std::initializer_list<TextureConfig> configs) {
    assert(configs.size() > 0);
    std::vector<TextureConfig> configVec(configs);
    addImageArrayBinding(2, 1, configVec);
    textureSetFlags |= TextureSetFlag::NORMAL_TEXTURE_SET;
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addMetallicRoughnessTextures(const std::initializer_list<TextureConfig> configs) {
    assert(configs.size() > 0);
    std::vector<TextureConfig> configVec(configs);
    addImageArrayBinding(2, 2, configVec);
    textureSetFlags |= TextureSetFlag::METALLIC_ROUGHNESS_SET;
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addAmbientOcclusionTextures(const std::initializer_list<TextureConfig> configs) {
    assert(configs.size() > 0);
    std::vector<TextureConfig> configVec(configs);
    addImageArrayBinding(2, 3, configVec);
    textureSetFlags |= TextureSetFlag::OCCLUSION_TEXTURE_SET;
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addEmissiveTextures(const std::initializer_list<TextureConfig> configs) {
    assert(configs.size() > 0);
    std::vector<TextureConfig> configVec(configs);
    addImageArrayBinding(2, 4, configVec);
    textureSetFlags |= TextureSetFlag::EMISSIVE_TEXTURE_SET;
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::setMetallicFactor(float f) {
    metallicFactor = f;
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::setRoughnessFactor(float f) {
    roughnessFactor = f;
    return *this;
}