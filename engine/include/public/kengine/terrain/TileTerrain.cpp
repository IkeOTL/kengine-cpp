#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>


float DualGridTileTerrain::getHeightAt(float x, float z) {
    return 0.0f;
}

TileTerrainChunkTile& DualGridTileTerrain::getTile(uint32_t x, uint32_t z) {

}

void DualGridTileTerrain::regenerate(VulkanContext& vkContext, AsyncModelCache& modelCache) {

}
