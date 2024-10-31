#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/terrain/TerrainContext.hpp>
#include <kengine/math.hpp>

#include <glm/glm.hpp>
#include <array>
#include <kengine/util/MatUtils.hpp>
#include <kengine/vulkan/pipelines/PreDrawCullingPipeline.hpp>
#include <kengine/vulkan/pipelines/TerrainDrawCullingPipeline.hpp>
#include <kengine/vulkan/pipelines/TerrainPreDrawCullingPipeline.hpp>

namespace ke {
    VkSemaphore CullContext::getSemaphore(size_t frameIdx) {
        return semaphores[frameIdx]->handle;
    }

    void CullContext::init(VulkanContext& vkCxt) {
        // create compute cmdbufs and sempahores
        for (size_t i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            computeCmdBufs[i] = vkCxt.getCommandPool()->createComputeCmdBuf();

            VkSemaphoreCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkSemaphore newSemaphore;
            VKCHECK(vkCreateSemaphore(vkCxt.getVkDevice(), &createInfo, nullptr, &newSemaphore),
                "Failed to create semaphore");

            semaphores[i] = ke::VulkanSemaphore::create(vkCxt.getVkDevice(), newSemaphore);
        }

        auto& descSetAllocators = vkCxt.getDescSetAllocators();
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
            auto descSetInit = [](size_t idx, VkDescriptorSet descSet, const CachedGpuBuffer& buf,
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

                {
                    auto cullingDescSet = descSetAllo.getGlobalDescriptorSet("pre-deferred-culling", PreDrawCullingPipeline::preCullingLayout);

                    std::vector<VkWriteDescriptorSet> setWrites(1);
                    std::vector<VkDescriptorBufferInfo> bufferInfos(1);

                    descSetInit(0, cullingDescSet, indirectBuf, setWrites, bufferInfos);

                    vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, VK_NULL_HANDLE);
                }

                {
                    auto cullingDescSet = descSetAllo.getGlobalDescriptorSet("terrain-pre-deferred-culling", TerrainPreDrawCullingPipeline::preCullingLayout);

                    std::vector<VkWriteDescriptorSet> setWrites(1);
                    std::vector<VkDescriptorBufferInfo> bufferInfos(1);

                    descSetInit(0, cullingDescSet, *terrainContext->getDrawIndirectBuf(), setWrites, bufferInfos);

                    vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, VK_NULL_HANDLE);
                }
            }
        }
    }

    void CullContext::dispatch(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, CameraController& cc, int frameIdx, int drawCallCount, int objectCount) {
        auto cmdBuf = computeCmdBufs[frameIdx]->vkCmdBuf;

        VkCommandBufferBeginInfo cmdBeginInfo{};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo);
        {
            // precull
            {
                // pre cull pass. reset the draw cmds to 0 instanceCount instead of doign it on the CPU
                auto localSizeX = 32; // must match compute shader
                auto localSizeY = 32; // must match compute shader

                // terrain
                if (terrainContext) {
                    auto chunkCount = terrainContext->getChunkCount().x * terrainContext->getChunkCount().y;
                    auto workGroupCountX = static_cast<uint32_t>((chunkCount + (localSizeX * localSizeY) - 1) / (localSizeX * localSizeY));
                    auto dispatchSizeX = static_cast<uint32_t>(ceil(sqrt(workGroupCountX)));
                    auto dispatchSizeY = static_cast<uint32_t>(ceil((float)workGroupCountX / dispatchSizeX));

                    auto pc = PreDrawCullingPipeline::PushConstant{};
                    pc.totalDrawCalls = chunkCount;

                    auto& pl = vkCxt.getPipelineCache().getPipeline<PreDrawCullingPipeline>();
                    pl.bind(vkCxt, descSetAllocator, cmdBuf, frameIdx);
                    vkCmdPushConstants(cmdBuf, pl.getVkPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PreDrawCullingPipeline::PushConstant), &pc);
                    vkCmdDispatch(cmdBuf, dispatchSizeX, dispatchSizeY, 1);
                }

                // main
                {
                    auto chunkCount = terrainContext->getChunkCount().x * terrainContext->getChunkCount().y;
                    auto workGroupCountX = static_cast<uint32_t>((chunkCount + (localSizeX * localSizeY) - 1) / (localSizeX * localSizeY));
                    auto dispatchSizeX = static_cast<uint32_t>(ceil(sqrt(workGroupCountX)));
                    auto dispatchSizeY = static_cast<uint32_t>(ceil((float)workGroupCountX / dispatchSizeX));

                    auto pc = TerrainPreDrawCullingPipeline::PushConstant{};
                    pc.totalChunkCount = chunkCount;

                    auto& pl = vkCxt.getPipelineCache().getPipeline<TerrainPreDrawCullingPipeline>();
                    pl.bind(vkCxt, descSetAllocator, cmdBuf, frameIdx);
                    vkCmdPushConstants(cmdBuf, pl.getVkPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PreDrawCullingPipeline::PushConstant), &pc);
                    vkCmdDispatch(cmdBuf, dispatchSizeX, dispatchSizeY, 1);
                }

                std::vector<VkBufferMemoryBarrier2> barriers;
                barriers.reserve(2);

                // terrain
                if (terrainContext) {
                    const auto& terrainIndirectBuf = terrainContext->getDrawIndirectBuf();
                    VkBufferMemoryBarrier2 bufferBarrier{};
                    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
                    bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                    bufferBarrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                    bufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                    bufferBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferBarrier.buffer = terrainIndirectBuf->getGpuBuffer().getVkBuffer();
                    bufferBarrier.offset = terrainIndirectBuf->getFrameOffset(frameIdx);
                    bufferBarrier.size = terrainIndirectBuf->getFrameSize();
                    barriers.push_back(bufferBarrier);
                }

                // main
                {
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
                    barriers.push_back(bufferBarrier);
                }

                VkDependencyInfo depInfo{};
                depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                depInfo.bufferMemoryBarrierCount = barriers.size();
                depInfo.pBufferMemoryBarriers = barriers.data();

                vkCmdPipelineBarrier2(cmdBuf, &depInfo);
            }

            // terrain culling
            if (terrainContext) {
                {
                    auto localSizeX = 32; // must match compute shader
                    auto localSizeY = 32; // must match compute shader
                    auto chunkCount = terrainContext->getChunkCount().x * terrainContext->getChunkCount().y;
                    auto workGroupCountX = static_cast<uint32_t>((chunkCount + (localSizeX * localSizeY) - 1) / (localSizeX * localSizeY));
                    auto dispatchSizeX = static_cast<uint32_t>(ceil(sqrt(workGroupCountX)));
                    auto dispatchSizeY = static_cast<uint32_t>(ceil((float)workGroupCountX / dispatchSizeX));

                    auto camera = cc.getCamera();
                    auto& proj = camera->getProjectionMatrix();
                    auto frustumX = matutils::frustumPlane(proj, matutils::PLANE_NX);
                    auto frustumY = matutils::frustumPlane(proj, matutils::PLANE_PX);

                    auto pc = TerrainDrawCullingPipeline::PushConstant{};
                    camera->getViewMatrix(pc.viewMatrix);
                    pc.frustum[0] = frustumX.x;
                    pc.frustum[1] = frustumX.z;
                    pc.frustum[2] = frustumY.x;
                    pc.frustum[3] = frustumY.z;
                    pc.sphereBounds = terrainContext->getChunkBoundingSphere(); // need to calculate this
                    pc.chunkDimensions = terrainContext->getChunkDimensions();
                    pc.chunkCount = terrainContext->getChunkCount();
                    pc.worldOffset = terrainContext->getWorldOffset();

                    auto& pl = vkCxt.getPipelineCache().getPipeline<TerrainDrawCullingPipeline>();
                    pl.bind(vkCxt, descSetAllocator, cmdBuf, frameIdx);
                    vkCmdPushConstants(cmdBuf, pl.getVkPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(TerrainDrawCullingPipeline::PushConstant), &pc);

                    vkCmdDispatch(cmdBuf, dispatchSizeX, dispatchSizeY, 1);
                }
            }

            // main object
            {
                auto localSizeX = 32; // must match compute shader
                auto localSizeY = 32; // must match compute shader
                auto workGroupCountX = static_cast<uint32_t>((objectCount + (localSizeX * localSizeY) - 1) / (localSizeX * localSizeY));
                auto dispatchSizeX = static_cast<uint32_t>(ceil(sqrt(workGroupCountX)));
                auto dispatchSizeY = static_cast<uint32_t>(ceil((float)workGroupCountX / dispatchSizeX));

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
                    std::vector<VkBufferMemoryBarrier2> barriers;
                    barriers.reserve(2);

                    {
                        VkBufferMemoryBarrier2 bufferBarrier{};
                        bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
                        bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                        bufferBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
                        bufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                        bufferBarrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                        bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        bufferBarrier.buffer = indirectBuf.getGpuBuffer().getVkBuffer();
                        bufferBarrier.offset = indirectBuf.getFrameOffset(frameIdx);
                        bufferBarrier.size = indirectBuf.getFrameSize();
                        barriers.push_back(bufferBarrier);
                    }

                    if (terrainContext) {
                        const auto& terrainIndirectBuf = terrainContext->getDrawIndirectBuf();
                        VkBufferMemoryBarrier2 terrainBarrier{};
                        terrainBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
                        terrainBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                        terrainBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
                        terrainBarrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                        terrainBarrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                        terrainBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        terrainBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        terrainBarrier.buffer = terrainIndirectBuf->getGpuBuffer().getVkBuffer();
                        terrainBarrier.offset = terrainIndirectBuf->getFrameOffset(frameIdx);
                        terrainBarrier.size = terrainIndirectBuf->getFrameSize();
                        barriers.push_back(terrainBarrier);
                    }

                    VkDependencyInfo depInfo{};
                    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    depInfo.bufferMemoryBarrierCount = barriers.size();
                    depInfo.pBufferMemoryBarriers = barriers.data();

                    vkCmdPipelineBarrier2(cmdBuf, &depInfo);
                }
            }
        }
        vkEndCommandBuffer(cmdBuf);

        VkCommandBufferSubmitInfo cmdBufSubmitInfo{};
        cmdBufSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdBufSubmitInfo.commandBuffer = cmdBuf;

        VkSemaphoreSubmitInfo semaInfo{};
        semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaInfo.semaphore = semaphores[frameIdx]->handle;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdBufSubmitInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &semaInfo;

        vkCxt.getComputeQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
    }
} // namespace ke