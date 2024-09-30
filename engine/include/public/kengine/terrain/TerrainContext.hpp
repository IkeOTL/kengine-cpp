#pragma once
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/vulkan/GpuUploadable.hpp>

#include <vector>
#include <memory>

class VulkanContext;
class CameraController;
class RenderPassContext;
class CachedGpuBuffer;
class AsyncMaterialCache;
class Material;

class TerrainContext {
public:
    class IndirectDrawCmdBuffer : public GpuUploadable {
    private:
        uint32_t frameSize;
        uint32_t indicesCount;

    public:
        IndirectDrawCmdBuffer(uint32_t frameSize, uint32_t indicesCount)
            : frameSize(frameSize), indicesCount(indicesCount) {}

        void upload(VulkanContext& vkCxt, void* data) override {
            VkDrawIndexedIndirectCommand cmd{};
            cmd.indexCount = indicesCount;

            auto* buf = static_cast<unsigned char*>(data);
            for (auto i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
                auto pos = frameSize * i;
                memcpy(buf + pos, &cmd, sizeof(VkDrawIndexedIndirectCommand));
            }
        }

        VkDeviceSize size() override {
            return static_cast<VkDeviceSize>(frameSize * VulkanContext::FRAME_OVERLAP);
        }
    };

private:
    std::unique_ptr<GpuBuffer> chunkIndicesBuf;
    CachedGpuBuffer* drawIndirectCmdBuf = nullptr;
    CachedGpuBuffer* terrainDataBuf = nullptr;
    CachedGpuBuffer* drawInstanceBuf = nullptr;
    CachedGpuBuffer* materialsBuf = nullptr;

    AsyncMaterialCache& materialCache;
    Material* material = nullptr;

    std::unique_ptr<TileTerrain> terrain;

public:
    inline static const int MAX_TILES = 1024 * 1024;
    inline static const int MAX_CHUNKS = 64 * 64;

    TerrainContext(AsyncMaterialCache& materialCache)
        : materialCache(materialCache) {}

    void init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators);

    void setMaterialBuf(CachedGpuBuffer* buf) {
        materialsBuf = buf;
    }

    void resetDrawBuf(uint32_t frameIdx);

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