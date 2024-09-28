#pragma once
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/terrain/TileTerrain.hpp>

#include <vector>
#include <memory>

class VulkanContext;
class CameraController;
class RenderPassContext;
class CachedGpuBuffer;

class TerrainContext {
private:
    std::unique_ptr<GpuBuffer> chunkIndicesBuf;
    CachedGpuBuffer* drawIndirectCmdBuf = nullptr;
    CachedGpuBuffer* terrainDataBuf = nullptr;
    CachedGpuBuffer* drawInstanceBuf = nullptr;
    CachedGpuBuffer& materialsBuf;

    std::unique_ptr<TileTerrain> terrain = nullptr;

public:
    inline static const int MAX_TILES = 1024 * 1024;
    inline static const int MAX_CHUNKS = 64 * 64;

    TerrainContext(CachedGpuBuffer& materialsBuf)
        : materialsBuf(materialsBuf) {}

    void init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators);

    void resetDrawBuf();

    //const glm::vec4& getChunkBoundingSphere();
    const glm::vec4 getChunkBoundingSphere();
    const glm::uvec2 getChunkDimensions();
    const glm::uvec2 getChunkCount();
    const glm::vec2 getWorldOffset();

    const CachedGpuBuffer* getDrawIndirectBuf() {
        return drawIndirectCmdBuf;
    }

    void draw(RenderPassContext& rpCtx);
    void cull(RenderPassContext& rpCtx);
};