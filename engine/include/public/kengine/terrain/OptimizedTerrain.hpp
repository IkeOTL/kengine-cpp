#pragma once
#include <kengine/terrain/TileTerrainChunk.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <kengine/vulkan/material/TerrainPbrMaterialConfig.hpp>

namespace ke {
    class VulkanContext;
    class AsyncModelCache;
    class MaterialConfig;

    struct TileData {
        uint32_t matIdx : 3 = 0; // 3 bits for material index (bits 0 to 2)
        uint32_t tileId : 12 = 0; // 12 bits for tile ID (bits 3 to 14)
        uint32_t unused : 17;

        void setTileId(uint16_t id) {
            tileId = id;
        }

        void setMatIdx(uint8_t idx) {
            matIdx = idx;
        }
    };

    static_assert(sizeof(TileData) == 4, "TileData size is not 32 bits.");

    class OptimizedTerrain {
    private:
        // sizes in "tiles"
        int terrainTilesWidth, terrainTilesLength;
        int terrainHeightsWidth, terrainHeightsLength;
        int chunkWidth, chunkLength;
        int chunkCountX, chunkCountZ;
        float worldOffsetX, worldOffsetZ;

        // how much of [0, 255] is considered 1unit in the world
        uint8_t unitSize;

        std::shared_ptr<TerrainPbrMaterialConfig> materialConfig;
        std::unique_ptr<Texture2d> heightTexture;

        std::vector<uint8_t> heights; // size in "verts"    
        std::vector<TileData> tileData; // sizes in "tiles"

    public:
        OptimizedTerrain(uint32_t terrainTilesWidth, uint32_t terrainTilesLength, uint32_t chunkWidth, uint32_t chunkLength, uint8_t unitSize = 10);

        float getHeightAt(float x, float z);


        const uint8_t getUnitSize() const {
            return unitSize;
        }

        uint32_t getTerrainTilesWidth() const {
            return terrainTilesWidth;
        }

        uint32_t getTerrainTilesLength() const {
            return terrainTilesLength;
        }

        uint32_t getTerrainHeightsWidth() const {
            return terrainHeightsWidth;
        }

        uint32_t getTerrainHeightsLength() const {
            return terrainHeightsLength;
        }

        uint32_t getChunkCountX() const {
            return chunkCountX;
        }

        uint32_t getChunkCountZ() const {
            return chunkCountZ;
        }

        uint32_t getChunkWidth() const {
            return chunkWidth;
        }

        uint32_t getChunkLength() const {
            return chunkLength;
        }

        float getWorldOffsetX() const {
            return worldOffsetX;
        }

        float getWorldOffsetZ() const {
            return worldOffsetZ;
        }

        const std::vector<uint8_t>& getHeights() const {
            return heights;
        }

        const std::vector<TileData>& getTileData() const {
            return tileData;
        }

        float getHeight(uint32_t x, uint32_t z) {
            return heights[z * terrainHeightsWidth + x] / unitSize;
        }

        /// <summary>
        /// will convert provided world height to "compressed" height (8 bits)
        /// hieght image is VK_FORMAT_R8_UNORM, so values are [0, 255] on CPU, and when reading in shader [0, 1]
        /// </summary>
        void setHeight(uint32_t x, uint32_t z, float h) {
            uint8_t target = h * unitSize;
            assert(target >= 0 && target <= 255);
            heights[z * terrainHeightsWidth + x] = target;
        }

        TileData& getTile(uint32_t x, uint32_t z) {
            return tileData[z * terrainTilesWidth + x];
        }

        Texture2d* getTerrainHeightImage() {
            return heightTexture.get();
        }

        TerrainPbrMaterialConfig* getMaterialConfig() {
            return materialConfig.get();
        }

        void setMaterialConfig(std::shared_ptr<TerrainPbrMaterialConfig> mConfig) {
            materialConfig = mConfig;
        }

        inline static float barryCentric(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec2& pos) {
            float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
            float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
            float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
            float l3 = 1.0f - l1 - l2;
            return l1 * p1.y + l2 * p2.y + l3 * p3.y;
        }
    };
} // namespace ke 