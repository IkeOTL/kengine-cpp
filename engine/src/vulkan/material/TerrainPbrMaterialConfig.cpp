#include <kengine/vulkan/material/TerrainPbrMaterialConfig.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/util/VecUtils.hpp>

#include <future>

void TerrainPbrMaterialConfig::upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) {
    auto mat = PbrMaterialData{
        albedoFactor,
        emissiveFactor,
        metallicFactor,
        roughnessFactor,
        0
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
    hash = prime * hash + albedoTextureSet;
    hash = prime * hash + metallicRoughnessTextureSet;
    hash = prime * hash + normalTextureSet;
    hash = prime * hash + occlusionTextureSet;
    hash = prime * hash + emissiveTextureSet;
    hash = prime * hash + static_cast<std::size_t>(std::bit_cast<int>(metallicFactor));
    hash = prime * hash + static_cast<std::size_t>(std::bit_cast<int>(roughnessFactor));
    return hash;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addAlbedoTexture(TextureConfig* config) {
  
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addNormalsTexture(TextureConfig* config) {
  
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addMetallicRoughnessTexture(TextureConfig* config) {
 
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addAmbientOcclusionTexture(TextureConfig* config) {
  
    return *this;
}

TerrainPbrMaterialConfig& TerrainPbrMaterialConfig::addEmissiveTexture(TextureConfig* config) {
   
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