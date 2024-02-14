#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/math.hpp>

#include <glm/glm.hpp>
#include <array>

VkSemaphore CullContext::getSemaphore(size_t frameIdx) {
    return semaphores[frameIdx];
}

void CullContext::init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators) {
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
            auto& descSetAllo = *descSetAllocators[i];

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
    auto cmdBuf = computeCmdBufs[frameIdx]->vkCmdBuf;

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo);
    {
        auto camera = cc.getCamera();
        auto& proj = camera->getProjectionMatrix();
        auto frustumX = math::frustumPlane(proj, math::PLANE_NX);
        auto frustumY = math::frustumPlane(proj, math::PLANE_PX);

        auto pc = DrawCullingPipeline::PushConstant{};
        camera->getViewMatrix(pc.viewMatrix);
        pc.frustum[0] = frustumX.x;
        pc.frustum[1] = frustumX.z;
        pc.frustum[2] = frustumY.x;
        pc.frustum[3] = frustumY.z;

        pc.p00 = proj[0][0];
        pc.p11 = proj[1][1];
        pc.zNear = camera->getNearClip();
        pc.zFar = camera->getFarClip();

        pc.totalInstances = objectCount;

        auto& pl = vkCxt.getPipelineCache().getPipeline<DrawCullingPipeline>();
        pl.bind(vkCxt, descSetAllocator, cmdBuf, frameIdx);
        vkCmdPushConstants(cmdBuf, pl.getVkPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DrawCullingPipeline::PushConstant), &pc);

        vkCmdDispatch(cmdBuf, (objectCount / 256) + 1, 1, 1);

        // insert barrier
        {
            VkMemoryBarrier2 barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

            VkDependencyInfo depInfo{};
            depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depInfo.pMemoryBarriers = &barrier;

            vkCmdPipelineBarrier2(cmdBuf, &depInfo);
        }
    }
    vkEndCommandBuffer(cmdBuf);

    VkCommandBufferSubmitInfo cmdBufSubmitInfo{};
    cmdBufSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmdBufSubmitInfo.commandBuffer = cmdBuf;

    VkSemaphoreSubmitInfo semaInfo{};
    semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    semaInfo.semaphore = semaphores[frameIdx];

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pCommandBufferInfos = &cmdBufSubmitInfo;
    submitInfo.pSignalSemaphoreInfos = &semaInfo;

    vkCxt.getComputeQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
}