#include <kengine/vulkan/IndirectDrawBatch.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/MaterialBindManager.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/material/Material.hpp>

namespace ke {
    void IndirectDrawBatch::predraw(VulkanContext& vkCxt, VkCommandBuffer vkCmd, MaterialBindManager& bindManager, DescriptorSetAllocator& descSetAllocator, int frameIdx) {
        bindManager.bind(*getMaterial(), descSetAllocator, vkCmd, frameIdx);
    }

    void IndirectDrawBatch::draw(VulkanContext& vkCxt, VkCommandBuffer vkCmd, CachedGpuBuffer& indirectCmdBuf, DescriptorSetAllocator& descSetAllocator, int frameIdx, MaterialBindManager& bindManager) {
        predraw(vkCxt, vkCmd, bindManager, descSetAllocator, frameIdx);

        VkDeviceSize offsets = 0;
        vkCmdBindVertexBuffers(vkCmd, 0, 1, &mesh->getVertexBuf().vkBuffer, &offsets);
        vkCmdBindIndexBuffer(vkCmd, mesh->getIndexBuf().vkBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexedIndirect(
            vkCmd,
            indirectCmdBuf.getGpuBuffer().getVkBuffer(),
            indirectCmdBuf.getFrameOffset(frameIdx) + cmdId * sizeof(VkDrawIndexedIndirectCommand),
            1,
            sizeof(VkDrawIndexedIndirectCommand));

        // postdraw();
    }
} // namespace ke
