#pragma once
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/terrain/OptimizedTerrain.hpp>

#include <vector>
#include <memory>

namespace ke {
    class VulkanContext;
    class CameraController;
    class RenderPassContext;
    class CachedGpuBuffer;
    class AsyncMaterialCache;
    class Material;

    class TerrainContext {
    private:
        CachedGpuBuffer* chunkIndicesBuf = nullptr;
        CachedGpuBuffer* drawIndirectCmdBuf = nullptr;
        CachedGpuBuffer* terrainDataBuf = nullptr;
        CachedGpuBuffer* drawInstanceBuf = nullptr;
        CachedGpuBuffer* materialsBuf = nullptr;

        std::unique_ptr<Texture2d> heightsTexture = nullptr;

        AsyncMaterialCache& materialCache;
        Material* material = nullptr;

        std::unique_ptr<OptimizedTerrain> terrain;

    public:
        inline static const int MAX_TILES = 1024 * 1024;
        inline static const int MAX_CHUNKS = 64 * 64;

        TerrainContext(AsyncMaterialCache& materialCache)
            : materialCache(materialCache) {}

        void init(VulkanContext& vkCxt);

        void setMaterialBuf(CachedGpuBuffer* buf) {
            materialsBuf = buf;
        }

        //const glm::vec4& getChunkBoundingSphere();
        const glm::vec4 getChunkBoundingSphere();
        const glm::uvec2 getChunkDimensions();
        const glm::uvec2 getChunkCount();
        const glm::vec2 getWorldOffset();

        const CachedGpuBuffer* getDrawIndirectBuf() {
            return drawIndirectCmdBuf;
        }

        static const size_t terrainDataBufAlignedFrameSize(VulkanContext& vkCxt);
        static const size_t drawInstanceBufAlignedFrameSize(VulkanContext& vkCxt);

        void draw(VulkanContext& vkCxt, RenderPassContext& rpCtx, DescriptorSetAllocator& descSetAllocator);
    };
} // namespace ke