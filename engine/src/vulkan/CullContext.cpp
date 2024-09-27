#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/math.hpp>

#include <glm/glm.hpp>
#include <array>
#include <kengine/util/MatUtils.hpp>
#include <kengine/vulkan/pipelines/PreDrawCullingPipeline.hpp>

VkSemaphore CullContext::getSemaphore(size_t frameIdx) {
    return semaphores[frameIdx];
}

void CullContext::init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators) {
    for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++)

        // create compute cmdbufs and sempahores
        for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            computeCmdBufs[i] = vkCxt.getCommandPool()->createComputeCmdBuf();

            VkSemaphoreCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

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

            auto cullingDescSet = descSetAllo.getGlobalDescriptorSet("deferred-culling", DrawCullingPipeline::cullingLayout);

            std::vector<VkWriteDescriptorSet> setWrites(4);
            std::vector<VkDescriptorBufferInfo> bufferInfos(4);

            descSetInit(0, cullingDescSet, indirectBuf, setWrites, bufferInfos);
            descSetInit(1, cullingDescSet, objectInstanceBuf, setWrites, bufferInfos);
            descSetInit(2, cullingDescSet, drawObjectBuf, setWrites, bufferInfos);
            descSetInit(3, cullingDescSet, drawInstanceBuffer, setWrites, bufferInfos);

            vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, VK_NULL_HANDLE);
        }
    }

    {
        auto descSetInit = [](size_t idx, VkDescriptorSet descSet, CachedGpuBuffer& buf,
            std::vector<VkWriteDescriptorSet>& setWrites, std::vector<VkDescriptorBufferInfo>& bufferInfos) {
                auto& binding = PreDrawCullingPipeline::preCullingLayout.bindings[idx];
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

            auto cullingDescSet = descSetAllo.getGlobalDescriptorSet("pre-deferred-culling", PreDrawCullingPipeline::preCullingLayout);

            std::vector<VkWriteDescriptorSet> setWrites(1);
            std::vector<VkDescriptorBufferInfo> bufferInfos(1);

            descSetInit(0, cullingDescSet, indirectBuf, setWrites, bufferInfos);

            vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, VK_NULL_HANDLE);
        }
    }
}

void CullContext::dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int drawCallCount, int objectCount) {
    auto cmdBuf = computeCmdBufs[frameIdx]->vkCmdBuf;

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo);
    {
        auto localSizeX = 32; // must match compute shader
        auto localSizeY = 32; // must match compute shader
        auto workGroupCountX = static_cast<uint32_t>((objectCount + (localSizeX * localSizeY) - 1) / (localSizeX * localSizeY));
        auto dispatchSizeX = static_cast<uint32_t>(ceil(sqrt(workGroupCountX)));
        auto dispatchSizeY = static_cast<uint32_t>(ceil((float)workGroupCountX / dispatchSizeX));

        // pre cull pass. reset the draw cmds to 0 instanceCount instead of doign it on the CPU
        {
            auto pc = PreDrawCullingPipeline::PushConstant{};
            pc.totalDrawCalls = drawCallCount;

            auto& pl = vkCxt.getPipelineCache().getPipeline<PreDrawCullingPipeline>();
            pl.bind(vkCxt, descSetAllocator, cmdBuf, frameIdx);
            vkCmdPushConstants(cmdBuf, pl.getVkPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PreDrawCullingPipeline::PushConstant), &pc);
            vkCmdDispatch(cmdBuf, dispatchSizeX, dispatchSizeY, 1);

            // Create a VkMemoryBarrier2 for synchronization between the two dispatches
             VkBufferMemoryBarrier2 bufferBarrier{};
             bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
             bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
             bufferBarrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
             bufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
             bufferBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
             bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
             bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
             bufferBarrier.buffer = indirectBuf.getGpuBuffer().getVkBuffer();
             bufferBarrier.offset = indirectBuf.getFrameOffset(frameIdx);
             bufferBarrier.size = indirectBuf.getFrameSize();

             VkDependencyInfo depInfo{};
             depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
             depInfo.bufferMemoryBarrierCount = 1;
             depInfo.pBufferMemoryBarriers = &bufferBarrier;

             vkCmdPipelineBarrier2(cmdBuf, &depInfo);
        }

        auto camera = cc.getCamera();
        auto& proj = camera->getProjectionMatrix();
        auto frustumX = matutils::frustumPlane(proj, matutils::PLANE_NX);
        auto frustumY = matutils::frustumPlane(proj, matutils::PLANE_PX);

        auto pc = DrawCullingPipeline::PushConstant{};
        camera->getViewMatrix(pc.viewMatrix);
        pc.frustum[0] = frustumX.x;
        pc.frustum[1] = frustumX.z;
        pc.frustum[2] = frustumY.x;
        pc.frustum[3] = frustumY.z;

        pc.totalInstances = objectCount;

        auto& pl = vkCxt.getPipelineCache().getPipeline<DrawCullingPipeline>();
        pl.bind(vkCxt, descSetAllocator, cmdBuf, frameIdx);
        vkCmdPushConstants(cmdBuf, pl.getVkPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DrawCullingPipeline::PushConstant), &pc);

        vkCmdDispatch(cmdBuf, dispatchSizeX, dispatchSizeY, 1);

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
            depInfo.memoryBarrierCount = 1;
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
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufSubmitInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &semaInfo;

    vkCxt.getComputeQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
}