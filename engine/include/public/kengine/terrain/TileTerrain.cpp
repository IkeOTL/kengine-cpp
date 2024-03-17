#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/terrain/TileTerrainChunk.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>


float DualGridTileTerrain::getHeightAt(float x, float z) {
    auto terrainX = x - worldOffsetX;
    auto terrainZ = z - worldOffsetZ;

    auto gridX = (int)std::floor(terrainX);
    auto gridZ = (int)std::floor(terrainZ);

    if (gridX >= terrainHeightsWidth - 1 || gridZ >= terrainHeightsLength - 1
        || gridX < 0 || gridZ < 0)
        return 0;

    // 1.0f is typically the number of world units per tile (i tihnk i cant remember)
    // but since our tiles are each one unit we neglect it 
    auto xCoord = std::fmod(terrainX, 1.0f) / 1.0f;
    auto zCoord = std::fmod(terrainZ, 1.0f) / 1.0f;

    if (xCoord <= (1 - zCoord))
        return barryCentric(glm::vec3(0, getHeight(gridX, gridZ), 0),
            glm::vec3(1, getHeight(gridX + 1, gridZ), 0),
            glm::vec3(0, getHeight(gridX, gridZ + 1), 1),
            glm::vec2(xCoord, zCoord));

    return barryCentric(glm::vec3(1, getHeight(gridX + 1, gridZ), 0),
        glm::vec3(1, getHeight(gridX + 1, gridZ + 1), 1),
        glm::vec3(0, getHeight(gridX, gridZ + 1), 1),
        glm::vec2(xCoord, zCoord));
}

TileTerrainChunkTile& DualGridTileTerrain::getTile(uint32_t x, uint32_t z) {
    auto chunkX = x / chunkWidth;
    auto chunkZ = z / chunkLength;

    auto tileX = x % chunkWidth;
    auto tileZ = z % chunkLength;

    return getChunk(chunkX, chunkZ).getTile(tileX, tileZ);
}

void DualGridTileTerrain::regenerate(VulkanContext& vkContext, AsyncModelCache& modelCache) {

}
