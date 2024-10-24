#include <kengine/terrain/TileTerrainChunk.hpp>
#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/util/Random.hpp>

namespace ke {
    TileTerrainChunk::TileTerrainChunk(TileTerrain& parent, uint32_t chunkIdxX, uint32_t chunkIdxZ)
        : parent(parent),
        chunkWidth(parent.getChunkWidth()), chunkLength(parent.getChunkLength()),
        chunkIdxX(chunkIdxX), chunkIdxZ(chunkIdxZ) {

        modelConfig = std::make_shared<ModelConfig>(
            "Custom:Terrain:" + std::to_string(chunkIdxX) + "| " + std::to_string(chunkIdxZ),
            VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS | VertexAttribute::TANGENTS
        );

        tiles.reserve(chunkWidth * chunkLength);
        for (int i = 0; i < chunkWidth * chunkLength; i++) {
            tiles.push_back(TileTerrainChunkTile(*this));
            tiles[i].textureTileId = static_cast<uint16_t>(random::randInt(0, 2));
            //tiles[z * chunkWidth + x].setTextureTileId((short) 0);        
        }
    }

    glm::vec2 TileTerrainChunk::getWorldOffset() {
        auto worldOffsetX = parent.getWorldOffsetX() + (chunkIdxX * chunkWidth);
        auto worldOffsetZ = parent.getWorldOffsetZ() + (chunkIdxZ * chunkLength);;
        return glm::vec2(worldOffsetX, worldOffsetZ);
    }

    void TileTerrainChunk::regenerate(VulkanContext& vkContext, AsyncModelCache& modelCache) {
        MeshBuilder<TexturedVertex> mb(VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS | VertexAttribute::TANGENTS);

        auto worldChunkIdxX = chunkIdxX * chunkWidth;
        auto worldChunkIdxZ = chunkIdxZ * chunkLength;

        auto worldOffsetX = parent.getWorldOffsetX() + (chunkIdxX * chunkWidth);
        auto worldOffsetZ = parent.getWorldOffsetZ() + (chunkIdxZ * chunkLength);

        /*
            verts     faces
           0----3    1---0  0
           |    |    |  / / |
           |    |    | / /  |
           1----2    2  1---2
         */
        for (int z = 0; z < chunkLength; z++) {
            for (int x = 0; x < chunkWidth; x++) {
                const auto& tile = getTile(x, z);

                TexturedVertex v0{};
                TexturedVertex v1{};
                TexturedVertex v2{};
                TexturedVertex v3{};

                v0.position = glm::vec3(
                    /* worldOffsetX + */x,
                    parent.getHeight(x + worldChunkIdxX, z + worldChunkIdxZ),
                    /* worldOffsetZ + */ z
                );
                v0.texCoords = glm::vec2(tileSheet->getUv(tile.textureTileId, TileSheet::TileCorner::TOP_LEFT));

                v1.position = glm::vec3(
                    /* worldOffsetX + */x,
                    parent.getHeight(x + worldChunkIdxX, z + worldChunkIdxZ + 1),
                    /* worldOffsetZ + */ z + 1
                );
                v1.texCoords = glm::vec2(tileSheet->getUv(tile.textureTileId, TileSheet::TileCorner::BOTTOM_LEFT));

                v2.position = glm::vec3(
                    /* worldOffsetX + */x + 1,
                    parent.getHeight(x + worldChunkIdxX + 1, z + worldChunkIdxZ + 1),
                    /* worldOffsetZ + */ z + 1
                );
                v2.texCoords = glm::vec2(tileSheet->getUv(tile.textureTileId, TileSheet::TileCorner::BOTTOM_RIGHT));

                v3.position = glm::vec3(
                    /* worldOffsetX + */x + 1,
                    parent.getHeight(x + worldChunkIdxX + 1, z + worldChunkIdxZ),
                    /* worldOffsetZ + */ +z
                );
                v3.texCoords = glm::vec2(tileSheet->getUv(tile.textureTileId, TileSheet::TileCorner::TOP_RIGHT));

                auto i0 = mb.pushVertex(std::move(v0));
                auto i1 = mb.pushVertex(std::move(v1));
                auto i2 = mb.pushVertex(std::move(v2));
                auto i3 = mb.pushVertex(std::move(v3));

                mb.pushTriangle(i3, i0, i1);
                mb.pushTriangle(i3, i1, i2);
            }
        }

        auto mesh = mb.build(&vkContext, true, true);
        modelCache.unsafeAdd(*modelConfig, std::make_unique<Model>(std::move(mesh)));
    }
} // namespace ke