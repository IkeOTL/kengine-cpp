#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/CommandBuffer.hpp>
#include <vector>

class VulkanContext;
class GpuBufferCache;
class CommandBuffer;
class CachedGpuBuffer;
class CameraController;

class CullContext {
private:
    std::unique_ptr<CommandBuffer> computeCmdBufs[VulkanContext::FRAME_OVERLAP];
    VkSemaphore semaphores[VulkanContext::FRAME_OVERLAP];

    CachedGpuBuffer& indirectBuf;
    CachedGpuBuffer& objectInstanceBuf;
    CachedGpuBuffer& drawObjectBuf;
    CachedGpuBuffer& drawInstanceBuffer;

public:
    VkSemaphore getSemaphore(size_t frameIdx);

    CullContext(CachedGpuBuffer& indirectBuf, CachedGpuBuffer& objectInstanceBuf, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer& drawInstanceBuffer)
        : indirectBuf(indirectBuf), objectInstanceBuf(objectInstanceBuf), drawObjectBuf(drawObjectBuf), drawInstanceBuffer(drawInstanceBuffer) {}

    void init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators);
    void dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int drawCallCount, int objectCount);
};