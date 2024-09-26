#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/terrain/TileTerrainChunk.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>

DualGridTileTerrain::DualGridTileTerrain(uint32_t terrainTilesWidth, uint32_t terrainTilesLength, uint32_t chunkWidth, uint32_t chunkLength)
    : terrainTilesWidth(terrainTilesWidth), terrainTilesLength(terrainTilesLength),
    chunkWidth(chunkWidth), chunkLength(chunkLength),
    terrainHeightsWidth(terrainTilesWidth + 2), terrainHeightsLength(terrainTilesLength + 2),
    worldOffsetX((-static_cast<float>(terrainTilesWidth) / 2.0f) - .5f), worldOffsetZ((-static_cast<float>(terrainTilesLength) / 2.0f) - .5f), // dual grid terrain: https://www.youtube.com/watch?v=Uxeo9c-PX-w&t=308s
    chunkCountX(terrainTilesWidth / chunkWidth), chunkCountZ(terrainTilesLength / chunkLength)
{
    if (terrainTilesWidth % 2 != 0 || terrainTilesLength % 2 != 0
        || terrainTilesWidth % chunkWidth != 0 || terrainTilesLength % chunkLength != 0)
        throw std::runtime_error("TileTerrain params must be even, and chunk size must be a factor of terrain size.");

    heights.resize(terrainHeightsWidth * terrainHeightsLength, 0);

    // temp tilesheet
    auto tileSheet = std::make_shared<TileSheet>(96, 80, 16, 16);
    chunks.reserve(chunkCountX * chunkCountZ);
    for (int z = 0; z < chunkCountZ; z++) {
        for (int x = 0; x < chunkCountX; x++) {
            chunks.push_back(std::make_unique<TileTerrainChunk>(*this, x, z));
            chunks[z * chunkCountX + x]->setTileSheet(tileSheet);
        }
    }
}

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
    for (int z = 0; z < chunkCountZ; z++) {
        for (int x = 0; x < chunkCountX; x++) {
            getChunk(x, z).regenerate(vkContext, modelCache);
        }
    }

    // generate texture
    //{
    //    auto hL = getTerrainHeightsWidth() * getTerrainHeightsLength();
    //    auto& h = getHeights();
    //    std::vector<float> hImageBbBuf;
    //    hImageBbBuf.reserve(hL);
    //    for (auto n = 0; n < hL; n++)
    //        hImageBbBuf.push_back(h[n]);

    //    heightTexture = std::make_unique<Texture2d>(
    //        vkContext,
    //        reinterpret_cast<const unsigned char*>(hImageBbBuf.data()),
    //        getTerrainHeightsWidth(),
    //        getTerrainHeightsLength(),
    //        VK_FORMAT_R32_SFLOAT,
    //        VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D,
    //        4, // 4 bytes per point, overkill
    //        VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
    //        false);
    //}
}
