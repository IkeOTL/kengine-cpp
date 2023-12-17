#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <vector>

class VulkanContext;
class GpuBufferCache;
class CommandBuffer;
class CachedGpuBuffer;
class CameraController;

namespace DescriptorSet {
    class DescriptorSetAllocator;
}

using namespace DescriptorSet;

class CullContext {
private:
    std::vector<std::unique_ptr<CommandBuffer>> computeCmdBufs;
    std::vector<VkSemaphore> semaphores;

    CachedGpuBuffer& indirectBuf;
    CachedGpuBuffer& objectInstanceBuf;
    CachedGpuBuffer& drawObjectBuf;
    CachedGpuBuffer& drawInstanceBuffer;

    void insertBarrier(VulkanContext& vkCxt, VkCommandBuffer cmdBuf, size_t frameIdx);

public:
    VkSemaphore getSemaphore(size_t frameIdx);

    void init(VulkanContext& vkCxt, std::vector<DescriptorSetAllocator>& descSetAllocators);
    void dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int objectCount);
};