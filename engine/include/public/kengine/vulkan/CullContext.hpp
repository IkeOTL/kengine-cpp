#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <vector>

class VulkanContext;
class GpuBufferCache;
class CommandBuffer;
class CachedGpuBuffer;
class CameraController;

class CullContext {
private:
    std::vector<std::unique_ptr<CommandBuffer>> computeCmdBufs;
    std::vector<VkSemaphore> semaphores;

    CachedGpuBuffer& indirectBuf;
    CachedGpuBuffer& objectInstanceBuf;
    CachedGpuBuffer& drawObjectBuf;
    CachedGpuBuffer& drawInstanceBuffer;

public:
    VkSemaphore getSemaphore(size_t frameIdx);

    CullContext(CachedGpuBuffer& indirectBuf, CachedGpuBuffer& objectInstanceBuf, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer& drawInstanceBuffer)
        : indirectBuf(indirectBuf), objectInstanceBuf(objectInstanceBuf), drawObjectBuf(drawObjectBuf), drawInstanceBuffer(drawInstanceBuffer) {}

    void init(VulkanContext& vkCxt, std::vector<DescriptorSetAllocator>& descSetAllocators);
    void dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int objectCount);
};