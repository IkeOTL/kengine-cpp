#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/VulkanContext.hpp>

#include <future>

void PbrMaterialConfig::upload(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, uint32_t frameIndex, int materialId) {
    do upload
}

size_t PbrMaterialConfig::hash() const noexcept {
    doo hash
        return size_t();
}

PbrMaterialConfig& PbrMaterialConfig::addAlbedoTexture(TextureConfig* config) {
    // TODO: insert return statement here
}

PbrMaterialConfig& PbrMaterialConfig::addNormalsTexture(TextureConfig* config) {
    // TODO: insert return statement here
}

PbrMaterialConfig& PbrMaterialConfig::addMetallicRoughnessTexture(TextureConfig* config) {
    // TODO: insert return statement here
}

PbrMaterialConfig& PbrMaterialConfig::addAmbientOcclusionTexture(TextureConfig* config) {
    // TODO: insert return statement here
}

PbrMaterialConfig& PbrMaterialConfig::addEmissiveTexture(TextureConfig* config) {
    // TODO: insert return statement here
}
