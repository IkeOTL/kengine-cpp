#include <kengine/vulkan/ShadowContext.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/IndirectDrawBatch.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/SamplerCache.hpp>

void ShadowContext::init(VulkanContext& vkContext, std::vector<DescriptorSetAllocator>& descSetAllocators,
    glm::vec3 lightDir, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer drawInstanceBuffer) {
    auto xferFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    shadowPassCascadeBuf = &bufCache.createHostMapped(
        ShadowCascadeData::SHADOW_CASCADE_COUNT * 16 * sizeof(float),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);

    compositePassCascadeBuf = &bufCache.createHostMapped(
        ShadowCascadeData::size(),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);

    cascadesData.setLightDir(lightDir);

    for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto& descSetAllocator = descSetAllocators[i];
        std::vector<VkWriteDescriptorSet> setWrites(4);

        {
            VkDescriptorSet modelMatDescriptorSet = descSetAllocator.getGlobalDescriptorSet(
                "shadow-pass0", CascadeShadowMapPipeline::shadowPassLayout);
            // Model matrix descriptor write
            auto& modelBufBinding = CascadeShadowMapPipeline::shadowPassLayout.getBinding(0);
            VkDescriptorBufferInfo modelBufferInfo{};
            modelBufferInfo.buffer = drawObjectBuf.getGpuBuffer().getVkBuffer();
            modelBufferInfo.offset = 0;
            modelBufferInfo.range = drawObjectBuf.getFrameSize();

            setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[0].dstSet = modelMatDescriptorSet;
            setWrites[0].dstBinding = modelBufBinding.bindingIndex;
            setWrites[0].descriptorCount = modelBufBinding.descriptorCount;
            setWrites[0].descriptorType = modelBufBinding.descriptorType;
            setWrites[0].pBufferInfo = &modelBufferInfo;

            // Draw instance descriptor write
            auto& drawInstanceBinding = CascadeShadowMapPipeline::shadowPassLayout.getBinding(1);
            VkDescriptorBufferInfo drawInstanceBufferInfo{};
            drawInstanceBufferInfo.buffer = drawInstanceBuffer.getGpuBuffer().getVkBuffer();
            drawInstanceBufferInfo.offset = 0;
            drawInstanceBufferInfo.range = drawInstanceBuffer.getFrameSize();

            setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[1].dstSet = modelMatDescriptorSet;
            setWrites[1].dstBinding = drawInstanceBinding.bindingIndex;
            setWrites[1].descriptorCount = drawInstanceBinding.descriptorCount;
            setWrites[1].descriptorType = drawInstanceBinding.descriptorType;
            setWrites[1].pBufferInfo = &drawInstanceBufferInfo;
        }

        {
            VkDescriptorSet cascadeDescriptorSet = descSetAllocator.getGlobalDescriptorSet(
                "cascade", CascadeShadowMapPipeline::cascadeViewProjLayout);
            // Cascade UBO descriptor write
            auto& cascadeUboBinding = CascadeShadowMapPipeline::cascadeViewProjLayout.getBinding(0);
            VkDescriptorBufferInfo cascadeBufferInfo{};
            cascadeBufferInfo.buffer = shadowPassCascadeBuf->getGpuBuffer().getVkBuffer(); // Assuming similar method exists in C++
            cascadeBufferInfo.offset = 0;
            cascadeBufferInfo.range = shadowPassCascadeBuf->getFrameSize();

            setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[2].dstSet = cascadeDescriptorSet;
            setWrites[2].dstBinding = cascadeUboBinding.bindingIndex;
            setWrites[2].descriptorCount = cascadeUboBinding.descriptorCount;
            setWrites[2].descriptorType = cascadeUboBinding.descriptorType;
            setWrites[2].pBufferInfo = &cascadeBufferInfo;
        }

        {
            VkDescriptorSet compositionDescriptorSet = descSetAllocator.getGlobalDescriptorSet(
                "deferred-composition", DeferredCompositionPbrPipeline::compositionLayout);
            // Composition descriptor write
            auto& cascadesUboBinding = DeferredCompositionPbrPipeline::compositionLayout.getBinding(7);
            VkDescriptorBufferInfo compositionBufferInfo{};
            compositionBufferInfo.buffer = compositePassCascadeBuf->getGpuBuffer().getVkBuffer(); // Assuming similar method exists in C++
            compositionBufferInfo.offset = 0;
            compositionBufferInfo.range = compositePassCascadeBuf->getFrameSize();

            setWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[3].dstSet = compositionDescriptorSet;
            setWrites[3].dstBinding = cascadesUboBinding.bindingIndex;
            setWrites[3].descriptorCount = cascadesUboBinding.descriptorCount;
            setWrites[3].descriptorType = cascadesUboBinding.descriptorType;
            setWrites[3].pBufferInfo = &compositionBufferInfo;
        }

        vkUpdateDescriptorSets(vkContext.getVkDevice(), static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

void ShadowContext::execute(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt, DescriptorSetAllocator& dAllocator,
    IndirectDrawBatch* nonSkinnedBatches, size_t nonSkinnedBatchesSize,
    IndirectDrawBatch* skinnedBatches, size_t skinnedBatchesSize) {
    const auto SHADOWDIM = 4096;

    auto camera = cameraController.getCamera();

    auto nearClip = camera->getNearClip();
    auto farClip = camera->getFarClip();
    auto clipRange = farClip - nearClip;

    auto minZ = nearClip;
    auto maxZ = nearClip + clipRange;

    auto range = maxZ - minZ;
    auto ratio = maxZ / minZ;

    glm::mat4 invCamViewProj{};
    camera->getViewMatrix(invCamViewProj);
    invCamViewProj = glm::inverse(camera->getProjectionMatrix() * invCamViewProj);

    auto cascadeSplitLambda = 0.75f;
    auto lastDistSplit = 0.0f;
    for (int i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {
        auto p = (i + 1.0f) / (float)ShadowCascadeData::SHADOW_CASCADE_COUNT;
        auto log = minZ * std::powf(ratio, p);
        auto uniform = minZ + range * p;
        auto d = cascadeSplitLambda * (log - uniform) + uniform;
        auto splitDist = (d - nearClip) / clipRange;

        auto& cascade = cascadesData.getCascade(i);
        cascade.updateViewProj(invCamViewProj, camera->getNearClip(), sceneData.getLightDir(),
            lastDistSplit, splitDist, clipRange);

        lastDistSplit = splitDist;
    }

    cascadesData.uploadShadowPass(vkContext, *shadowPassCascadeBuf, cxt.frameIndex);
    cascadesData.uploadCompositionPass(vkContext, *compositePassCascadeBuf, cxt.frameIndex);

    for (int i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {
        auto rp1Cxt = RenderPass::RenderPassContext{ 1, i, cxt.cmd, glm::uvec2(SHADOWDIM) };
        vkContext.beginRenderPass(rp1Cxt);
        {
            auto& pipeline = vkContext.getPipelineCache().getPipeline<CascadeShadowMapPipeline>();
            execShadowPass(vkContext, cxt, pipeline, dAllocator, i, nonSkinnedBatches, nonSkinnedBatchesSize, false);

            auto& skinnedPipeline = vkContext.getPipelineCache().getPipeline<SkinnedCascadeShadowMapPipeline>();
            execShadowPass(vkContext, cxt, skinnedPipeline, dAllocator, i, skinnedBatches, skinnedBatchesSize, true);
        }
        vkContext.endRenderPass(rp1Cxt);
    }
}

void ShadowContext::execShadowPass(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt,
    Pipeline& p1, DescriptorSetAllocator& dAllocator, size_t cascadeIdx,
    IndirectDrawBatch* batches, size_t batchesSize, bool skinned) {
    if (batchesSize == 0)
        return;

    auto vkCmd = cxt.cmd;
    p1.bind(vkContext, dAllocator, vkCmd, cxt.frameIndex);

    vkCmdPushConstants(vkCmd, p1.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(cascadeIdx), &cascadeIdx);

    auto indCmdFrameOffset = indirectCmdBuf.getFrameOffset(cxt.frameIndex);
    for (auto i = 0; i < batchesSize; i++) {
        auto& indirectBatch = batches[i];

        {
            // Extracting texture and sampler setup
            auto& bindingTexture = static_cast<ImageBinding&>(indirectBatch.getMaterial()->getBinding(2, 0)).getTexture();

            // move to material code
            auto samplerConfig = SamplerConfig(
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                VK_FILTER_NEAREST,
                VK_FILTER_NEAREST,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_COMPARE_OP_NEVER,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                0,
                bindingTexture.getMipLevels(),
                0,
                1.0f);
            auto sampler = vkContext.getSamplerCache().getSampler(samplerConfig);

            auto layout = skinned ? SkinnedCascadeShadowMapPipeline::skinnedSingleTextureLayout : CascadeShadowMapPipeline::textureLayout;
            auto pTexSet = dAllocator.leaseDescriptorSet(layout);

            VkDescriptorImageInfo descImgBufInfo = {};
            descImgBufInfo.sampler = sampler;
            descImgBufInfo.imageView = bindingTexture.getImageView();
            descImgBufInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            std::vector<VkWriteDescriptorSet> setWrites(skinned ? 2 : 1);

            auto& textureBinding = layout.getBinding(0);
            VkWriteDescriptorSet& textureWrite = setWrites[0];
            textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            textureWrite.dstBinding = textureBinding.bindingIndex;
            textureWrite.dstSet = pTexSet;
            textureWrite.descriptorCount = textureBinding.descriptorCount;
            textureWrite.descriptorType = textureBinding.descriptorType;
            textureWrite.pImageInfo = &descImgBufInfo;

            std::vector<uint32_t> offsets = { 0 };

            if (skinned) {
                auto& bindingSkele = static_cast<BufferBinding&>(indirectBatch.getMaterial()->getBinding(2, 5)).getGpuBuffer();
                auto& skeletonBinding = layout.getBinding(1);
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = bindingSkele.getGpuBuffer().getVkBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range = bindingSkele.getFrameSize();

                VkWriteDescriptorSet& skeletonWrite = setWrites[1];
                skeletonWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                skeletonWrite.dstBinding = skeletonBinding.bindingIndex;
                skeletonWrite.dstSet = pTexSet;
                skeletonWrite.descriptorCount = skeletonBinding.descriptorCount;
                skeletonWrite.descriptorType = skeletonBinding.descriptorType;
                skeletonWrite.pBufferInfo = &bufferInfo;

                offsets[0] = bindingSkele.getFrameOffset(cxt.frameIndex);
            }

            vkUpdateDescriptorSets(vkContext.getVkDevice(), static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);

            vkCmdBindDescriptorSets(
                cxt.cmd,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                p1.getVkPipelineLayout(),
                2,
                1,
                &pTexSet,
                offsets.size(),
                offsets.data()
            );
        }

        auto mesh = indirectBatch.getMesh();
        VkBuffer vertexBuffers[] = { mesh->getVertexBuf().getVkBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vkCmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(vkCmd, mesh->getIndexBuf().getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexedIndirect(
            vkCmd,
            indirectCmdBuf.getGpuBuffer().getVkBuffer(),
            indCmdFrameOffset + indirectBatch.getCmdId() * sizeof(VkDrawIndexedIndirectCommand),
            1,
            sizeof(VkDrawIndexedIndirectCommand)
        );
    }
}
