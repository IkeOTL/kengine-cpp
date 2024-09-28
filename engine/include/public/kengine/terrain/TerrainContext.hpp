#pragma once
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>

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
    CachedGpuBuffer* materialsBuf = nullptr;

public:
    inline static const int MAX_TILES = 1024 * 1024;
    inline static const int MAX_CHUNKS = 64 * 64;

    void init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators);

    void draw(RenderPassContext& rpCtx);
};