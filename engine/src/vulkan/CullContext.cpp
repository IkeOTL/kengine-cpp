#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>

VkSemaphore CullContext::getSemaphore(size_t frameIdx) {
    return semaphores[frameIdx];
}

void CullContext::init(VulkanContext& vkCxt, std::vector<DescriptorSetAllocator>& descSetAllocators) {
    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++)
        computeCmdBufs[i] = vkCxt.getCommandPool()->createComputeCmdBuf();

    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;

        VKCHECK(vkCreateSemaphore(vkCxt.getVkDevice(), &createInfo, nullptr, &semaphores[i]),
            "Failed to create semaphore");
    }

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

        std::vector<VkWriteDescriptorSet> setWrites(4);
        std::vector<VkDescriptorBufferInfo> bufferInfos(4);

        auto cullingDescSet = descSetAllo.getGlobalDescriptorSet(
            "deferred-culling", DrawCullingPipeline::cullingLayout);

        descSetInit(0, cullingDescSet, indirectBuf, setWrites, bufferInfos);
        descSetInit(1, cullingDescSet, objectInstanceBuf, setWrites, bufferInfos);
        descSetInit(2, cullingDescSet, drawObjectBuf, setWrites, bufferInfos);
        descSetInit(3, cullingDescSet, drawInstanceBuffer, setWrites, bufferInfos);

        vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, VK_NULL_HANDLE);
    }
}

void CullContext::dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int objectCount) {

}

void CullContext::insertBarrier(VulkanContext& vkCxt, VkCommandBuffer cmdBuf, size_t frameIdx) {

}

static void descSetInit(size_t idx, VkDescriptorSet descSet, CachedGpuBuffer& buf,
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
}