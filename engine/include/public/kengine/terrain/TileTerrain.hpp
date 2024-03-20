#pragma once
#include <kengine/terrain/TileTerrainChunk.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class VulkanContext;
class AsyncModelCache;
class MaterialConfig;
class Texture2d;

class TileTerrain {
public:
    /// <returns>the width of the terrains in tiles</returns>
    virtual uint32_t getTerrainTilesWidth() = 0;

    /// <returns>the width of the terrains height map</returns>
    virtual uint32_t getTerrainTilesLength() = 0;

    /// <returns>the width of the terrains height map</returns>
    virtual uint32_t getTerrainHeightsWidth() = 0;

    /// <returns>the width of the terrains height map</returns>
    virtual uint32_t getTerrainHeightsLength() = 0;

    virtual uint32_t getChunkCountX() = 0;
    virtual uint32_t getChunkCountZ() = 0;
    virtual uint32_t getChunkWidth() = 0;
    virtual uint32_t getChunkLength() = 0;
    virtual float getWorldOffsetX() = 0;
    virtual float getWorldOffsetZ() = 0;
    virtual std::vector<float>& getHeights() = 0;
    virtual float getHeightFromWorld(int32_t x, int32_t z) = 0;
    virtual void setHeightFromWorld(int32_t x, int32_t z, float h) = 0;
    virtual float getHeight(uint32_t x, uint32_t z) = 0;
    virtual void setHeight(uint32_t x, uint32_t z, float h) = 0;
    virtual float getHeightAt(float x, float z) = 0;
    virtual TileTerrainChunk& getChunk(uint32_t x, uint32_t z) = 0;
    virtual TileTerrainChunkTile& getTileFromWorld(int32_t x, int32_t z) = 0;
    virtual TileTerrainChunkTile& getTile(uint32_t x, uint32_t z) = 0;
    virtual Texture2d* getTerrainHeightImage() = 0;
    virtual MaterialConfig* getMaterialConfig() = 0;
    virtual void setMaterialConfig(std::shared_ptr<MaterialConfig> mConfig) = 0;
    virtual void regenerate(VulkanContext& vkContext, AsyncModelCache& modelCache) = 0;
};

class DualGridTileTerrain : public TileTerrain {
private:
    // sizes in "tiles"
    int terrainTilesWidth, terrainTilesLength;
    int terrainHeightsWidth, terrainHeightsLength;
    int chunkWidth, chunkLength;
    int chunkCountX, chunkCountZ;
    float worldOffsetX, worldOffsetZ;

    std::vector<std::unique_ptr<TileTerrainChunk>> chunks;

    std::shared_ptr<MaterialConfig> materialConfig;
    std::unique_ptr<Texture2d> heightTexture;

    // size in "verts"
    std::vector<float> heights;

public:
    DualGridTileTerrain(uint32_t terrainTilesWidth, uint32_t terrainTilesLength, uint32_t chunkWidth, uint32_t chunkLength);


    float getHeightAt(float x, float z) override;
    TileTerrainChunkTile& getTile(uint32_t x, uint32_t z) override;
    void regenerate(VulkanContext& vkContext, AsyncModelCache& modelCache) override;

    uint32_t getTerrainTilesWidth() override {
        return terrainTilesWidth;
    }

    uint32_t getTerrainTilesLength() override {
        return terrainTilesLength;
    }

    uint32_t getTerrainHeightsWidth() override {
        return terrainHeightsWidth;
    }

    uint32_t getTerrainHeightsLength() override {
        return terrainHeightsLength;
    }

    uint32_t getChunkCountX() override {
        return chunkCountX;
    }

    uint32_t getChunkCountZ() override {
        return chunkCountZ;
    }

    uint32_t getChunkWidth() override {
        return chunkWidth;
    }

    uint32_t getChunkLength() override {
        return chunkLength;
    }

    float getWorldOffsetX() override {
        return worldOffsetX;
    }

    float getWorldOffsetZ() override {
        return worldOffsetZ;
    }

    std::vector<float>& getHeights() override {
        return heights;
    }

    /// not supported for dual grid terrain
    float getHeightFromWorld(int32_t x, int32_t z) override {
        throw std::runtime_error("Not supported for this terrain type");
    }

    /// not supported for dual grid terrain
    void setHeightFromWorld(int32_t x, int32_t z, float h) override {}

    float getHeight(uint32_t x, uint32_t z) override {
        return heights[z * terrainHeightsWidth + x];
    }

    void setHeight(uint32_t x, uint32_t z, float h) override {
        heights[z * terrainHeightsWidth + x] = h;
    }

    TileTerrainChunk& getChunk(uint32_t x, uint32_t z) override {
        return *chunks[z * chunkCountX + x];
    }

    /// not supported for dual grid terrain
    TileTerrainChunkTile& getTileFromWorld(int32_t x, int32_t z) override {
        throw std::runtime_error("Not supported for this terrain type");
    }

    Texture2d* getTerrainHeightImage() override {
        return heightTexture.get();
    }

    MaterialConfig* getMaterialConfig() override {
        return materialConfig.get();
    }

    void setMaterialConfig(std::shared_ptr<MaterialConfig> mConfig) override {
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