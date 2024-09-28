#pragma once
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>

#include <vector>
#include <memory>

class VulkanContext;

class TerrainContext {
private:
    std::unique_ptr<GpuBuffer> chunkIndicesBuf;
    CachedGpuBuffer* drawIndirectCmdBuf;
    CachedGpuBuffer* terrainDataBuf;
    CachedGpuBuffer* drawInstanceBuf;

public:
    inline static const int MAX_TILES = 1024 * 1024;
    inline static const int MAX_CHUNKS = 64 * 64;

    void init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators);
};