#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/SceneData.hpp>
#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/ShadowContext.hpp>
#include <kengine/vulkan/material/Material.hpp>
#include <kengine/vulkan/DrawObjectBuffer.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/SamplerCache.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>

void RenderContext::init() {
    for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto ptr = std::make_unique<DescriptorSetAllocator>(vkContext.getVkDevice(), vkContext.getDescSetLayoutCache());
        descSetAllocators.push_back(std::move(ptr));
    }

    auto lightDir = glm::normalize(glm::vec3(-.05f, -.4f, -.2f));
    sceneData = std::make_unique<SceneData>(cameraController, lightDir);

    initBuffers();
    initDescriptors();

    cullContext = std::make_unique<CullContext>(*indirectCmdBuf, *objectInstanceBuf,
        *drawObjectBuf, *drawInstanceBuffer);
    cullContext->init(vkContext, descSetAllocators);

    shadowContext = std::make_unique<ShadowContext>(bufCache, *indirectCmdBuf, cameraController, *sceneData);
    shadowContext->init(vkContext, descSetAllocators, lightDir, *drawObjectBuf, *drawInstanceBuffer);
}

void RenderContext::setViewportScissor(glm::uvec2 dim) {
    auto vkCmd = frameCxt->cmd;
    auto vp = VkViewport{
        0, 0,
        static_cast<float>(dim.x), static_cast<float>(dim.y),
        0, 1
    };
    vkCmdSetViewport(vkCmd, 0, 1, &vp);

    auto rect = VkRect2D{
        {0, 0},
        {dim.x, dim.y}
    };
    vkCmdSetScissor(vkCmd, 0, 1, &rect);
}

void RenderContext::initBuffers() {
    auto xferFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        //| VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    sceneBuf = &bufCache.createHostMapped(
        SceneData::size(),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);
    lightBuf = &bufCache.createHostMapped(
        LightsManager::size(),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);
    indirectCmdBuf = &bufCache.createHostMapped(
        MAX_INSTANCES * sizeof(VkDrawIndexedIndirectCommand),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);
    objectInstanceBuf = &bufCache.createHostMapped(
        MAX_INSTANCES * sizeof(ObjectInstance),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);

    materialsBuf = &bufCache.createHostMapped(
        MaterialsBuffer::size(),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);
    drawObjectBuf = &bufCache.createHostMapped(
        DrawObjectBuffer::frameSize(),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);

    drawInstanceBuffer = &bufCache.create(
        MAX_INSTANCES * sizeof(uint32_t),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
}

void RenderContext::initDescriptors() {
    for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto& descSetAllocator = descSetAllocators[i];

        std::vector<VkWriteDescriptorSet> setWrites(7);
        std::vector<VkDescriptorBufferInfo> bufferInfos(6);
        VkDescriptorImageInfo imageInfo{};

        // Scene data
        {
            auto globalDescriptorSet = descSetAllocator->getGlobalDescriptorSet("deferred-global-layout", PipelineCache::globalLayout);
            auto& sceneDataBinding = PipelineCache::globalLayout.getBinding(0);

            bufferInfos[0].buffer = sceneBuf->getGpuBuffer().vkBuffer;
            bufferInfos[0].offset = 0;
            bufferInfos[0].range = sceneBuf->getFrameSize();

            setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[0].dstSet = globalDescriptorSet;
            setWrites[0].dstBinding = sceneDataBinding.bindingIndex;
            setWrites[0].descriptorCount = sceneDataBinding.descriptorCount;
            setWrites[0].descriptorType = sceneDataBinding.descriptorType;
            setWrites[0].pBufferInfo = &bufferInfos[0];
        }

        // model data
        {
            auto modelMatDescriptorSet = descSetAllocator->getGlobalDescriptorSet("deferred-gbuffer", DeferredOffscreenPbrPipeline::objectLayout);

            {
                auto& modelBufBinding = DeferredOffscreenPbrPipeline::objectLayout.getBinding(0);

                bufferInfos[1].buffer = drawObjectBuf->getGpuBuffer().vkBuffer;
                bufferInfos[1].offset = 0;
                bufferInfos[1].range = drawObjectBuf->getFrameSize();

                setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                setWrites[1].dstSet = modelMatDescriptorSet;
                setWrites[1].dstBinding = modelBufBinding.bindingIndex;
                setWrites[1].descriptorCount = modelBufBinding.descriptorCount;
                setWrites[1].descriptorType = modelBufBinding.descriptorType;
                setWrites[1].pBufferInfo = &bufferInfos[1];
            }

            {
                auto& drawInstanceBinding = DeferredOffscreenPbrPipeline::objectLayout.getBinding(1);

                bufferInfos[2].buffer = drawInstanceBuffer->getGpuBuffer().vkBuffer;
                bufferInfos[2].offset = 0;
                bufferInfos[2].range = drawInstanceBuffer->getFrameSize();

                setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                setWrites[2].dstSet = modelMatDescriptorSet;
                setWrites[2].dstBinding = drawInstanceBinding.bindingIndex;
                setWrites[2].descriptorCount = drawInstanceBinding.descriptorCount;
                setWrites[2].descriptorType = drawInstanceBinding.descriptorType;
                setWrites[2].pBufferInfo = &bufferInfos[2];
            }

            {
                auto& materialsBinding = DeferredOffscreenPbrPipeline::objectLayout.getBinding(2);
                bufferInfos[3].buffer = materialsBuf->getGpuBuffer().vkBuffer;
                bufferInfos[3].offset = 0;
                bufferInfos[3].range = materialsBuf->getFrameSize();

                setWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                setWrites[3].dstSet = modelMatDescriptorSet;
                setWrites[3].dstBinding = materialsBinding.bindingIndex;
                setWrites[3].descriptorCount = materialsBinding.descriptorCount;
                setWrites[3].descriptorType = materialsBinding.descriptorType;
                setWrites[3].pBufferInfo = &bufferInfos[3];
            }
        }

        auto compositionDescriptorSet = descSetAllocator->getGlobalDescriptorSet("deferred-composition", DeferredCompositionPbrPipeline::compositionLayout);
        // dynamic lights buf
        {
            auto& lightsUboBinding = DeferredCompositionPbrPipeline::compositionLayout.getBinding(5);
            bufferInfos[4].buffer = lightBuf->getGpuBuffer().vkBuffer;
            bufferInfos[4].offset = 0;
            bufferInfos[4].range = lightBuf->getFrameSize();

            setWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[4].dstSet = compositionDescriptorSet;
            setWrites[4].dstBinding = lightsUboBinding.bindingIndex;
            setWrites[4].descriptorCount = lightsUboBinding.descriptorCount;
            setWrites[4].descriptorType = lightsUboBinding.descriptorType;
            setWrites[4].pBufferInfo = &bufferInfos[4];
        }

        // shadows
        {
            auto samplerConfig = SamplerConfig(
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_COMPARE_OP_NEVER,
                VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                0,
                1,
                0,
                1.0f
            );
            auto shadowSampler = vkContext.getSamplerCache().getSampler(samplerConfig);

            auto& rp = vkContext.getRenderPass<CascadeShadowMapRenderPass>(1);
            auto& rt = rp.getRenderTarget<CascadeShadowMapRenderTarget>(i);

            auto& binding = DeferredCompositionPbrPipeline::compositionLayout.getBinding(6);
            imageInfo.sampler = shadowSampler;
            imageInfo.imageView = rt.getShadowMapDepthImage().imageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            setWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[5].dstSet = compositionDescriptorSet;
            setWrites[5].dstBinding = binding.bindingIndex;
            setWrites[5].descriptorCount = binding.descriptorCount;
            setWrites[5].descriptorType = binding.descriptorType;
            setWrites[5].pImageInfo = &imageInfo;
        }

        // dynamic materials buf
        {
            auto& binding = DeferredCompositionPbrPipeline::compositionLayout.getBinding(8);
            bufferInfos[5].buffer = materialsBuf->getGpuBuffer().vkBuffer;
            bufferInfos[5].offset = 0;
            bufferInfos[5].range = materialsBuf->getFrameSize();

            setWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrites[6].dstSet = compositionDescriptorSet;
            setWrites[6].dstBinding = binding.bindingIndex;
            setWrites[6].descriptorCount = binding.descriptorCount;
            setWrites[6].descriptorType = binding.descriptorType;
            setWrites[6].pBufferInfo = &bufferInfos[4];
        }

        vkUpdateDescriptorSets(vkContext.getVkDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
    }
}

void RenderContext::addStaticInstance(Mesh& mesh, Material& material, glm::mat4 transform, glm::vec4 boundingSphere, boolean hasShadow) {
    auto instanceIdx = staticInstances++;

    auto& draw = getStaticBatch(instanceIdx, mesh, material, hasShadow);

    for (auto frameIdx = 0; frameIdx < VulkanContext::FRAME_OVERLAP; frameIdx++) {
        // upload model specific details
        {
            // material: todo: only upload once per material id
            material.upload(vkContext, *materialsBuf, frameIdx);

            auto obj = DrawObject{
                transform,
                boundingSphere,
                material.getId()
            };

            auto modelBufStartPos = (int)drawObjectBuf->getFrameOffset(frameIdx);
            auto pos = modelBufStartPos + (instanceIdx * sizeof(DrawObject));
            auto buf = static_cast<unsigned char*>(drawObjectBuf->getGpuBuffer().data());
            memcpy(buf + pos, &obj, sizeof(DrawObject));
        }

        // buf upload   
        {
            auto objInstance = ObjectInstance{
                draw.getCmdId(),
                instanceIdx
            };

            auto objInstanceStartPos = (int)objectInstanceBuf->getFrameOffset(frameIdx);
            auto pos = objInstanceStartPos + (instanceIdx * sizeof(ObjectInstance));
            auto buf = static_cast<unsigned char*>(objectInstanceBuf->getGpuBuffer().data());
            memcpy(buf + pos, &objInstance, sizeof(ObjectInstance));
        }
    }
}

int RenderContext::draw(Mesh& mesh, Material& material, glm::mat4 transform, glm::vec4 boundingSphere, boolean hasShadow, boolean hasSkeleton) {
    auto frameIdx = frameCxt->frameIndex;
    auto instanceIdx = staticInstances + dynamicInstances++;

    // upload model specific details
    {
        // material: todo: only upload once per material id
        material.upload(vkContext, *materialsBuf, frameIdx);

        auto obj = DrawObject{
            transform,
            boundingSphere,
            material.getId()
        };

        auto modelBufStartPos = (int)drawObjectBuf->getFrameOffset(frameIdx);
        auto pos = modelBufStartPos + (instanceIdx * sizeof(DrawObject));
        auto buf = static_cast<unsigned char*>(drawObjectBuf->getGpuBuffer().data());
        memcpy(buf + pos, &obj, sizeof(DrawObject));
    }

    auto objInstanceStartPos = (int)objectInstanceBuf->getFrameOffset(frameIdx);
    auto curObjInstancePos = objInstanceStartPos + (instanceIdx * sizeof(ObjectInstance));

    // first batch
    if (dynamicBatches == 0) {
        auto batchIdx = dynamicBatches++;

        auto& newDraw = dynamicBatchCache[batchIdx];
        newDraw.reset();

        newDraw.setMesh(&mesh);
        newDraw.setMaterial(&material);
        newDraw.setCmdId(batchIdx + staticBatches);
        newDraw.setFirstInstanceIdx(instanceIdx);

        if (hasShadow) {
            if (hasSkeleton)
                shadowSkinnedBatchCache[totalShadowSkinnedBatches++] = newDraw;
            else
                shadowNonSkinnedBatchCache[staticShadowNonSkinnedBatches + totalShadowNonSkinnedBatches++] = newDraw;
        }

        // buf upload
        {
            auto objInstance = ObjectInstance{
                newDraw.getCmdId(),
                instanceIdx
            };

            auto buf = static_cast<unsigned char*>(objectInstanceBuf->getGpuBuffer().data());
            memcpy(buf + curObjInstancePos, &objInstance, sizeof(ObjectInstance));
        }

        return instanceIdx;
    }

    // check last instance can batch with this one
    {
        auto& last = dynamicBatchCache[dynamicBatches - 1];
        auto sameMesh = &mesh == last.getMesh();
        auto sameMaterial = &material == last.getMaterial();

        if (sameMesh && sameMaterial) {
            // buf upload
            auto objInstance = ObjectInstance{
                last.getCmdId(),
                instanceIdx
            };

            auto buf = static_cast<unsigned char*>(objectInstanceBuf->getGpuBuffer().data());
            memcpy(buf + curObjInstancePos, &objInstance, sizeof(ObjectInstance));

            return instanceIdx;
        }
    }

    // starting new batch
    {
        auto batchIdx = dynamicBatches++;
        auto& newDraw = dynamicBatchCache[batchIdx];
        newDraw.reset();

        newDraw.setMesh(&mesh);
        newDraw.setMaterial(&material);
        newDraw.setCmdId(batchIdx + staticBatches);
        newDraw.setFirstInstanceIdx(instanceIdx);

        if (hasShadow) {
            if (hasSkeleton)
                shadowSkinnedBatchCache[totalShadowSkinnedBatches++] = newDraw;
            else
                shadowNonSkinnedBatchCache[staticShadowNonSkinnedBatches + totalShadowNonSkinnedBatches++] = newDraw;
        }

        // buf upload
        {
            auto objInstance = ObjectInstance{
                newDraw.getCmdId(),
                instanceIdx
            };

            auto buf = static_cast<unsigned char*>(objectInstanceBuf->getGpuBuffer().data());
            memcpy(buf + curObjInstancePos, &objInstance, sizeof(ObjectInstance));
        }

        return instanceIdx;
    }
}

void RenderContext::begin(RenderFrameContext& frameCxt, float sceneTime, float alpha) {
    this->frameCxt = &frameCxt;
    this->sceneTime = sceneTime;
    this->alpha = alpha;

    dynamicInstances = 0;
    dynamicBatches = 0;
    totalShadowSkinnedBatches = 0;
    totalShadowNonSkinnedBatches = 0;

    bindManager.reset();

    started = true;
}

void RenderContext::end() {
    if (!started)
        throw std::runtime_error("RenderContext was never started.");

    auto frameIdx = frameCxt->frameIndex;

    sceneData->upload(vkContext, *sceneBuf, sceneTime, alpha, frameIdx);
    lightsManager.upload(vkContext, *lightBuf, frameIdx);

    static constexpr float invIndCmdSize = 1.0f / sizeof(VkDrawIndexedIndirectCommand);

    // reset static obj draw cmd
    {
        auto indCmdFrameOffset = static_cast<uint32_t>(indirectCmdBuf->getFrameOffset(frameIdx));
        auto indCmdFrameIdx = static_cast<uint32_t>(indCmdFrameOffset * invIndCmdSize);
        auto buf = indirectCmdBuf->getGpuBuffer().data();
        auto commands = static_cast<VkDrawIndexedIndirectCommand*>(buf);
        for (auto i = 0; i < staticBatches; i++) {
            auto& indirectBatch = staticBatchCache[i];
            // must review this!!!
            // could be the cause of rendering issues
            // havent tested yet
            auto cmdIdx = indCmdFrameIdx + indirectBatch.getCmdId();
            auto& indirectCmd = commands[cmdIdx];

            indirectCmd.firstInstance = indirectBatch.getFirstInstanceIdx();
            indirectCmd.instanceCount = 0;
            indirectCmd.firstIndex = 0;
            indirectCmd.indexCount = indirectBatch.getMesh()->getIndexCount();
        }
    }

    // upload batched dynamic draw cmds
    {
        auto indCmdFrameOffset = static_cast<uint32_t>(indirectCmdBuf->getFrameOffset(frameIdx));
        auto indCmdFrameIdx = static_cast<uint32_t>(indCmdFrameOffset * invIndCmdSize);
        auto buf = indirectCmdBuf->getGpuBuffer().data();
        auto commands = static_cast<VkDrawIndexedIndirectCommand*>(buf);
        for (auto i = 0; i < dynamicBatches; i++) {
            auto& indirectBatch = dynamicBatchCache[i];
            // must review this!!!
            // could be the cause of rendering issues
            // havent tested yet
            auto cmdIdx = indCmdFrameIdx + indirectBatch.getCmdId();
            auto& indirectCmd = commands[cmdIdx];

            indirectCmd.firstInstance = indirectBatch.getFirstInstanceIdx();
            indirectCmd.instanceCount = 0;
            indirectCmd.firstIndex = 0;
            indirectCmd.indexCount = indirectBatch.getMesh()->getIndexCount();
        }
    }

    // submit to GPU
    {
        frameCxt->cullComputeSemaphore = cullContext->getSemaphore(frameIdx);
        cullContext->dispatch(vkContext, *descSetAllocators[frameIdx], cameraController, frameIdx, staticInstances + dynamicInstances);

        vkContext.renderBegin(frameCxt);
        {
            auto& dAllocator = descSetAllocators[frameIdx];
            dAllocator.reset();

            shadowContext->execute(vkContext, *frameCxt, *dAllocator,
                shadowNonSkinnedBatchCache, staticShadowNonSkinnedBatches + totalShadowNonSkinnedBatches,
                shadowSkinnedBatchCache, totalShadowSkinnedBatches);

            deferredPass(*dAllocator);
        }
        vkContext.renderEnd(frameCxt);
    }

    started = false;
}

IndirectDrawBatch& RenderContext::getStaticBatch(int instanceIdx, Mesh& mesh, Material& material, boolean hasShadow) {
    if (staticBatches == 0) {
        auto batchIdx = staticBatches++;

        auto& newDraw = staticBatchCache[batchIdx];
        newDraw.reset();

        newDraw.setMesh(&mesh);
        newDraw.setMaterial(&material);
        newDraw.setCmdId(batchIdx);
        newDraw.setFirstInstanceIdx(instanceIdx);

        if (hasShadow)
            shadowNonSkinnedBatchCache[staticShadowNonSkinnedBatches++] = newDraw;

        return newDraw;
    }

    // check check last instance can batch with this one
    {
        auto& last = staticBatchCache[staticBatches - 1];
        auto sameMesh = &mesh == last.getMesh();
        auto sameMaterial = &material == last.getMaterial();

        if (sameMesh && sameMaterial)
            return last;
    }

    // starting new batch
    {
        auto batchIdx = staticBatches++;
        auto& newDraw = staticBatchCache[batchIdx];
        newDraw.reset();

        newDraw.setMesh(&mesh);
        newDraw.setMaterial(&material);
        newDraw.setCmdId(batchIdx);
        newDraw.setFirstInstanceIdx(instanceIdx);

        if (hasShadow)
            shadowNonSkinnedBatchCache[staticShadowNonSkinnedBatches++] = newDraw;

        return newDraw;
    }
}

void RenderContext::deferredPass(DescriptorSetAllocator& descSetAllocator) {
    auto frameIdx = frameCxt->frameIndex;
    auto vkCmd = frameCxt->cmd;

    auto rpCxt = RenderPassContext{ 0, frameIdx, vkCmd, frameCxt->swapchainExtents };
    vkContext.beginRenderPass(rpCxt);
    {
        setViewportScissor(rpCxt.extents);

        // subpass 1
        {
            for (int i = 0; i < staticBatches; i++) {
                auto& batch = staticBatchCache[i];
                batch.draw(vkContext, vkCmd, indirectCmdBuf->getGpuBuffer().getVkBuffer(),
                    descSetAllocator, frameIdx, bindManager);
            }

            for (int i = 0; i < dynamicBatches; i++) {
                auto& batch = dynamicBatchCache[i];
                batch.draw(vkContext, vkCmd, indirectCmdBuf->getGpuBuffer().getVkBuffer(),
                    descSetAllocator, frameIdx, bindManager);
            }
        }

        // subpass 2
        compositionSubpass(rpCxt, descSetAllocator);

        // subpass 3 forward transparency pass, TODO
        vkCmdNextSubpass(vkCmd, VK_SUBPASS_CONTENTS_INLINE);

        //guiManager.subpass(vkContext, rpCxt, frameCxt, descSetAllocator);
        // since guimanager isnt implemented yet we need to push to next subpass manually
        // we'll remove this line once the guimanager is working
        vkCmdNextSubpass(vkCmd, VK_SUBPASS_CONTENTS_INLINE);
    }
    vkContext.endRenderPass(rpCxt);
}

void RenderContext::compositionSubpass(RenderPassContext& rpCxt, DescriptorSetAllocator& d) {
    {
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
            1,
            0,
            1.0f);
        auto sampler = vkContext.getSamplerCache().getSampler(samplerConfig);

        auto& rp = vkContext.getRenderPass<DeferredPbrRenderPass>(rpCxt.renderPassIndex);
        auto& rt = rp.getRenderTarget<DeferredPbrRenderTarget>(rpCxt.renderTargetIndex);

        std::vector<VkDescriptorImageInfo> deskImgInfos = {
            VkDescriptorImageInfo {
                sampler,
                rt.getAlbedoImage().imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo {
                sampler,
                rt.getPositionImage().imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo {
                sampler,
                rt.getNormalImage().imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo {
                sampler,
                rt.getOrmImage().imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo {
                sampler,
                rt.getEmissiveImage().imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        };

        auto& layout = DeferredCompositionPbrPipeline::compositionLayout;
        auto pDescSet = d.getGlobalDescriptorSet("deferred-composition", layout);

        std::vector<uint32_t> bindingIdxs = { 0, 1, 2, 3, 4 };
        std::vector<VkWriteDescriptorSet> pWriteDesc(bindingIdxs.size());
        for (auto i = 0; i < bindingIdxs.size(); i++) {
            auto& binding = layout.getBinding(bindingIdxs[i]);
            auto& setWrite = pWriteDesc[i];
            setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrite.dstSet = pDescSet;
            setWrite.dstBinding = binding.bindingIndex;
            setWrite.descriptorCount = binding.descriptorCount;
            setWrite.descriptorType = binding.descriptorType;
            setWrite.pImageInfo = &deskImgInfos[i];
        }

        vkUpdateDescriptorSets(vkContext.getVkDevice(), pWriteDesc.size(), pWriteDesc.data(), 0, nullptr);
    }

    auto frameIdx = frameCxt->frameIndex;
    auto vkCmd = frameCxt->cmd;

    vkCmdNextSubpass(vkCmd, VK_SUBPASS_CONTENTS_INLINE);

    auto& pipeline = vkContext.getPipelineCache().getPipeline<DeferredCompositionPbrPipeline>();
    pipeline.bind(vkContext, d, vkCmd, frameIdx);

    vkCmdDraw(vkCmd, 3, 1, 0, 0);
}
