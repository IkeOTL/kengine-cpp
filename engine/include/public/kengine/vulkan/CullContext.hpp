#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/CommandBuffer.hpp>
#include <vector>
#include "VulkanObject.hpp"

namespace ke {
    class VulkanContext;
    class TerrainContext;
    class GpuBufferCache;
    class CommandBuffer;
    class CachedGpuBuffer;
    class CameraController;

    class CullContext {
    private:
        std::unique_ptr<CommandBuffer> computeCmdBufs[VulkanContext::FRAME_OVERLAP];
        std::unique_ptr<ke::VulkanSemaphore> semaphores[VulkanContext::FRAME_OVERLAP]{};

        TerrainContext* terrainContext = nullptr;
        CachedGpuBuffer& indirectBuf;
        CachedGpuBuffer& objectInstanceBuf;
        CachedGpuBuffer& drawObjectBuf;
        CachedGpuBuffer& drawInstanceBuffer;

    public:
        VkSemaphore getSemaphore(size_t frameIdx);

        CullContext(CachedGpuBuffer& indirectBuf, CachedGpuBuffer& objectInstanceBuf, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer& drawInstanceBuffer)
            : indirectBuf(indirectBuf), objectInstanceBuf(objectInstanceBuf), drawObjectBuf(drawObjectBuf), drawInstanceBuffer(drawInstanceBuffer) {}

        void setTerrainContext(TerrainContext* ctx) {
            terrainContext = ctx;
        }

        void init(VulkanContext& vkCxt);
        void dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int drawCallCount, int objectCount);
    };
} // namespace ke