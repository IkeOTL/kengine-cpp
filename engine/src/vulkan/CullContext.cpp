#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>

VkSemaphore CullContext::getSemaphore(size_t frameIdx) {
    return semaphores[frameIdx];
}

void CullContext::init(VulkanContext& vkCxt, std::vector<DescriptorSetAllocator>& descSetAllocators) {
    // allocate compute cmdbufs
    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++)
        computeCmdBufs[i] = vkCxt.getCommandPool()->createComputeCmdBuf();

    // create sempahores
    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;

        VKCHECK(vkCreateSemaphore(vkCxt.getVkDevice(), &createInfo, nullptr, &semaphores[i]),
            "Failed to create semaphore");
    }

    // init descriptorsets with buffers
    {
        auto descSetInit = [](size_t idx, VkDescriptorSet descSet, CachedGpuBuffer& buf,
            std::vector<VkWriteDescriptorSet>& setWrites, std::vector<VkDescriptorBufferInfo>& bufferInfos) {
                auto& binding = DrawCullingPipeline::cullingLayout.bindings[idx];
                auto& write = setWrites[idx];
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descSet;
                write.dstBinding = binding.bindingIndex;
                write.descriptorCount = binding.descriptorCount;
                write.descriptorType = binding.descriptorType;

                auto& bufferInfo = bufferInfos[idx];
                bufferInfo.buffer = buf.getGpuBuffer().getVkBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range = buf.getFrameSize();
                write.pBufferInfo = &bufferInfo;
        };

        for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            auto& descSetAllo = descSetAllocators[i];

            auto cullingDescSet = descSetAllo.getGlobalDescriptorSet(
                "deferred-culling", DrawCullingPipeline::cullingLayout);

            std::vector<VkWriteDescriptorSet> setWrites(4);
            std::vector<VkDescriptorBufferInfo> bufferInfos(4);

            descSetInit(0, cullingDescSet, indirectBuf, setWrites, bufferInfos);
            descSetInit(1, cullingDescSet, objectInstanceBuf, setWrites, bufferInfos);
            descSetInit(2, cullingDescSet, drawObjectBuf, setWrites, bufferInfos);
            descSetInit(3, cullingDescSet, drawInstanceBuffer, setWrites, bufferInfos);

            vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, VK_NULL_HANDLE);
        }
    }
}

void CullContext::dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int objectCount) {

}

void CullContext::insertBarrier(VulkanContext& vkCxt, VkCommandBuffer cmdBuf, size_t frameIdx) {
    VkMemoryBarrier2KHR barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR;
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR;

    VkDependencyInfoKHR depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    depInfo.pMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2KHR(cmdBuf, &depInfo);
}