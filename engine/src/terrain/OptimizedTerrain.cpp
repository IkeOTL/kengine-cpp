#include <kengine/terrain/OptimizedTerrain.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/util/Random.hpp>

OptimizedTerrain::OptimizedTerrain(uint32_t terrainTilesWidth, uint32_t terrainTilesLength,
    uint32_t chunkWidth, uint32_t chunkLength, uint8_t unitScale)
    : terrainTilesWidth(terrainTilesWidth), terrainTilesLength(terrainTilesLength),
    chunkWidth(chunkWidth), chunkLength(chunkLength),
    terrainHeightsWidth(terrainTilesWidth + 2), terrainHeightsLength(terrainTilesLength + 2), // adding 2 instead of 1 allows for .5tile offset to be fully centered
    worldOffsetX((-static_cast<float>(terrainTilesWidth) / 2.0f) - .5f), worldOffsetZ((-static_cast<float>(terrainTilesLength) / 2.0f) - .5f), // dual grid terrain: https://www.youtube.com/watch?v=Uxeo9c-PX-w&t=308s
    chunkCountX(terrainTilesWidth / chunkWidth), chunkCountZ(terrainTilesLength / chunkLength),
    unitScale(unitScale)
{
    if (terrainTilesWidth % 2 != 0 || terrainTilesLength % 2 != 0
        || terrainTilesWidth % chunkWidth != 0 || terrainTilesLength % chunkLength != 0)
        throw std::runtime_error("TileTerrain params must be even, and chunk size must be a factor of terrain size.");

    heights.resize(terrainHeightsWidth * terrainHeightsLength, 0);
    tileData.resize(terrainTilesWidth * terrainTilesLength);
}

float OptimizedTerrain::getHeightAt(float x, float z) {
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