#pragma once
#include <kengine/terrain/TileSheet.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <kengine/vulkan/mesh/ModelConfig.hpp>
#include <memory>

class TileTerrain;
class TileSheet;
class VulkanContext;
class AsyncModelCache;

class TileTerrainChunk;

struct TileTerrainChunkTile {
    TileTerrainChunk& parent;
    uint16_t textureTileId;

    TileTerrainChunkTile(TileTerrainChunk& parent) : parent(parent) {}
};

class TileTerrainChunk {
private:
    TileTerrain& parent;
    uint32_t chunkWidth, chunkLength;
    uint32_t chunkIdxX, chunkIdxZ;
    std::vector<TileTerrainChunkTile> tiles;
    std::shared_ptr<TileSheet> tileSheet;

    std::shared_ptr<ModelConfig> modelConfig;

public:
    TileTerrainChunk(TileTerrain& parent, uint32_t chunkIdxX, uint32_t chunkIdxZ);

    std::shared_ptr<ModelConfig> getModelConfig() {
        return modelConfig;
    }

    std::shared_ptr<TileSheet> getTileSheet() {
        return tileSheet;
    }

    void setTileSheet(std::shared_ptr<TileSheet> t) {
        tileSheet = t;
    }

    TileTerrainChunkTile& getTile(int x, int z) {
        return tiles[z * chunkWidth + x];
    }

    glm::vec2 getWorldOffset();
    void regenerate(VulkanContext& vkContext, AsyncModelCache& modelCache);
};