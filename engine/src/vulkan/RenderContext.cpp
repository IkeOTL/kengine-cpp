#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/SceneData.hpp>
#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/ImGuiKEContext.hpp>
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
#include <tracy/Tracy.hpp>
#include <kengine/vulkan/pipelines/DebugDeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>

void RenderContext::init(TerrainContext* terrainContext) {
    auto lightDir = glm::normalize(glm::vec3(-.05f, -.4f, -.2f));
    sceneData = std::make_unique<SceneData>(cameraController, lightDir);

    initBuffers();
    initDescriptors();

    // move?
    this->terrainContext = terrainContext;
    terrainContext->setMaterialBuf(materialsBuf);
    terrainContext->init(vkContext);

    cullContext = std::make_unique<CullContext>(*indirectCmdBuf, *objectInstanceBuf,
        *drawObjectBuf, *drawInstanceBuffer);
    cullContext->setTerrainContext(terrainContext);
    cullContext->init(vkContext);


    auto& bufCache = vkContext.getGpuBufferCache();
    shadowContext = std::make_unique<ShadowContext>(bufCache, *indirectCmdBuf, cameraController, *sceneData);
    shadowContext->init(vkContext, lightDir, *drawObjectBuf, *drawInstanceBuffer);
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

    auto& bufCache = vkContext.getGpuBufferCache();
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

// simplify this further
void RenderContext::initDescriptors() {

    // move somewhere else
#ifdef KE_DEBUG_RENDER
    {
        auto mb = MeshBuilder<DebugVertex>(VertexAttribute::POSITION);

        //0
        auto v0 = mb.createVertex();
        v0.position = { -.5f, .5f, -.5f };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        auto v2 = mb.createVertex();
        v2.position = { -.5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        auto v1 = mb.createVertex();
        v1.position = { .5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        v0 = mb.createVertex();
        v0.position = { .5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { -.5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { .5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        //1
        v0 = mb.createVertex();
        v0.position = { .5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v1 = mb.createVertex();
        v1.position = { .5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));
        v2 = mb.createVertex();
        v2.position = { -.5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));

        v0 = mb.createVertex();
        v0.position = { -.5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v1 = mb.createVertex();
        v1.position = { .5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));
        v2 = mb.createVertex();
        v2.position = { -.5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));

        //2
        v0 = mb.createVertex();
        v0.position = { -.5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { -.5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { .5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        v0 = mb.createVertex();
        v0.position = { -.5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { .5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { .5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        //3
        v0 = mb.createVertex();
        v0.position = { .5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { .5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { .5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        v0 = mb.createVertex();
        v0.position = { .5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { .5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { .5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        //4
        v0 = mb.createVertex();
        v0.position = { .5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { -.5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { -.5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        v0 = mb.createVertex();
        v0.position = { .5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { .5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { -.5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        //5
        v0 = mb.createVertex();
        v0.position = { -.5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { -.5, .5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { -.5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        v0 = mb.createVertex();
        v0.position = { -.5, .5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v0)));
        v2 = mb.createVertex();
        v2.position = { -.5, -.5, -.5 };
        mb.pushIndex(mb.pushVertex(std::move(v2)));
        v1 = mb.createVertex();
        v1.position = { -.5, -.5, .5 };
        mb.pushIndex(mb.pushVertex(std::move(v1)));

        debugMesh = mb.build(&vkContext);
    }
#endif

    auto& descSetAllocators = vkContext.getDescSetAllocators();
    for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto& descSetAllocator = descSetAllocators[i];

        std::vector<VkWriteDescriptorSet> setWrites;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;

        // so they dont resize
        setWrites.reserve(7);
        bufferInfos.reserve(6);
        imageInfos.reserve(1);

        auto pushBuf = [&](VkDescriptorSet vkDescSet, const DescriptorSetLayoutBindingConfig& bindingCfg, CachedGpuBuffer* gpuBuf) -> void {
            auto& buf = bufferInfos.emplace_back(VkDescriptorBufferInfo{
                    gpuBuf->getGpuBuffer().vkBuffer,
                    0,
                    gpuBuf->getFrameSize()
                });

            setWrites.emplace_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    vkDescSet,
                    bindingCfg.bindingIndex,
                    0,
                    bindingCfg.descriptorCount,
                    bindingCfg.descriptorType,
                    nullptr,
                    &buf,
                    nullptr
                });
            };

        auto pushImg = [&](VkDescriptorSet vkDescSet, const DescriptorSetLayoutBindingConfig& bindingCfg, const GpuImageView& gpuImg, VkSampler sampler) -> void {
            auto& img = imageInfos.emplace_back(VkDescriptorImageInfo{
                    sampler,
                    gpuImg.imageView,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                });

            setWrites.emplace_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    vkDescSet,
                    bindingCfg.bindingIndex,
                    0,
                    bindingCfg.descriptorCount,
                    bindingCfg.descriptorType,
                    &img,
                    nullptr,
                    nullptr
                });
            };

        // Scene data
        pushBuf(
            descSetAllocator->getGlobalDescriptorSet("deferred-global-layout", PipelineCache::globalLayout),
            PipelineCache::globalLayout.getBinding(0),
            sceneBuf
        );

        // model data
        {
            auto modelMatDescriptorSet = descSetAllocator->getGlobalDescriptorSet("deferred-gbuffer", DeferredOffscreenPbrPipeline::objectLayout);

            pushBuf(
                modelMatDescriptorSet,
                DeferredOffscreenPbrPipeline::objectLayout.getBinding(0),
                drawObjectBuf
            );

            pushBuf(
                modelMatDescriptorSet,
                DeferredOffscreenPbrPipeline::objectLayout.getBinding(1),
                drawInstanceBuffer
            );

            pushBuf(
                modelMatDescriptorSet,
                DeferredOffscreenPbrPipeline::objectLayout.getBinding(2),
                materialsBuf
            );
        }

        {
            auto compositionDescriptorSet = descSetAllocator->getGlobalDescriptorSet("deferred-composition", DeferredCompositionPbrPipeline::compositionLayout);

            // dynamic lights buf
            pushBuf(
                compositionDescriptorSet,
                DeferredCompositionPbrPipeline::compositionLayout.getBinding(5),
                lightBuf
            );

            // dynamic materials buf
            pushBuf(
                compositionDescriptorSet,
                DeferredCompositionPbrPipeline::compositionLayout.getBinding(8),
                materialsBuf
            );

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

                pushImg(
                    compositionDescriptorSet,
                    DeferredCompositionPbrPipeline::compositionLayout.getBinding(6),
                    rt.getShadowMapDepthImage(),
                    shadowSampler
                );
            }
        }

        vkUpdateDescriptorSets(vkContext.getVkDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
    }
}

uint32_t RenderContext::startStaticBatch() const {
    return staticBatches;
}

void RenderContext::endStaticBatch(uint32_t startBatchIndex) {
    ZoneScopedN("RenderContext::end - Upload Static Draw Cmds");

    if (startBatchIndex == staticBatches)
        return;

    for (auto fIdx = 0; fIdx < VulkanContext::FRAME_OVERLAP; fIdx++)
    {
        auto indCmdFrameOffset = static_cast<uint32_t>(indirectCmdBuf->getFrameOffset(fIdx));
        auto indCmdFrameIdx = static_cast<uint32_t>(indCmdFrameOffset * invIndCmdSize);
        auto buf = indirectCmdBuf->getGpuBuffer().data();
        auto* commands = static_cast<VkDrawIndexedIndirectCommand*>(buf);
        for (auto i = startBatchIndex; i < staticBatches; i++) {
            auto& indirectBatch = staticBatchCache[i];
            auto cmdIdx = indCmdFrameIdx + indirectBatch.getCmdId();
            auto& indirectCmd = commands[cmdIdx];

            indirectCmd.firstInstance = indirectBatch.getFirstInstanceIdx();
            indirectCmd.instanceCount = 0;
            indirectCmd.firstIndex = 0;
            indirectCmd.indexCount = indirectBatch.getMesh()->getIndexCount();
        }
    }
}


void RenderContext::addStaticInstance(const Mesh& mesh, const Material& material, const glm::mat4& transform, const glm::vec4& boundingSphere) {
    auto instanceIdx = staticInstances++;

    auto& draw = getStaticBatch(instanceIdx, mesh, material);

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

#ifdef KE_DEBUG_RENDER
void RenderContext::drawDebug(const glm::mat4& transform, const glm::vec4& color) {
    if (totalDebugObjects >= MAX_DEBUG_OBJECTS)
        return;

    debugObjects[totalDebugObjects++] = DebugObject{
        transform,
        color
    };
}
#endif

int RenderContext::draw(const Mesh& mesh, const Material& material, const glm::mat4& transform, const glm::vec4& boundingSphere) {
    ZoneScoped;
    auto frameIdx = frameCxt->frameIndex;
    uint32_t instanceIdx = staticInstances + dynamicInstances++;

    auto hasShadow = material.hasShadow();
    auto hasSkeleton = material.hasSkeleton();


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

        // upload skeleton here instead?
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
    ZoneScoped;
    this->frameCxt = &frameCxt;
    this->sceneTime = sceneTime;
    this->alpha = alpha;

    dynamicInstances = 0;
    dynamicBatches = 0;
    totalShadowSkinnedBatches = 0;
    totalShadowNonSkinnedBatches = 0;

#ifdef KE_DEBUG_RENDER
    totalDebugObjects = 0;
#endif

    bindManager.reset();

    started = true;
}

void RenderContext::end() {
    ZoneScoped;

    if (!started)
        throw std::runtime_error("RenderContext was never started.");

    auto frameIdx = frameCxt->frameIndex;

    sceneData->upload(vkContext, *sceneBuf, sceneTime, alpha, frameIdx);
    lightsManager.upload(vkContext, *lightBuf, alpha, frameIdx);

    // upload batched dynamic draw cmds
    {
        ZoneScopedN("RenderContext::end - Upload Dynamic Draw Cmds");

        auto indCmdFrameOffset = static_cast<uint32_t>(indirectCmdBuf->getFrameOffset(frameIdx));
        auto indCmdFrameIdx = static_cast<uint32_t>(indCmdFrameOffset * invIndCmdSize);
        auto buf = indirectCmdBuf->getGpuBuffer().data();
        auto commands = static_cast<VkDrawIndexedIndirectCommand*>(buf);
        for (auto i = 0; i < dynamicBatches; i++) {
            auto& indirectBatch = dynamicBatchCache[i];
            auto cmdIdx = indCmdFrameIdx + indirectBatch.getCmdId();
            auto& indirectCmd = commands[cmdIdx];

            indirectCmd.firstInstance = indirectBatch.getFirstInstanceIdx();
            indirectCmd.instanceCount = 0;
            indirectCmd.firstIndex = 0;
            indirectCmd.indexCount = indirectBatch.getMesh()->getIndexCount();
        }
    }

    auto& descSetAllocators = vkContext.getDescSetAllocators();
    // submit to GPU
    {
        ZoneScopedN("RenderContext::end - Submit to GPU");

        {
            ZoneScopedN("RenderContext::end - Compute Culling Dispatch");
            frameCxt->cullComputeSemaphore = cullContext->getSemaphore(frameIdx);
            // only up to static batches no need to reset dynamic draw cmds. we do that every frame already
            cullContext->dispatch(vkContext, *descSetAllocators[frameIdx], cameraController, frameIdx, staticBatches, staticInstances + dynamicInstances);
        }

        vkContext.renderBegin(*frameCxt);
        {
            auto& dAllocator = descSetAllocators[frameIdx];
            dAllocator->reset();

            // run this at same time?
            shadowContext->execute(vkContext, *frameCxt, *dAllocator,
                shadowNonSkinnedBatchCache, staticShadowNonSkinnedBatches + totalShadowNonSkinnedBatches,
                shadowSkinnedBatchCache, totalShadowSkinnedBatches);

            deferredPass(*dAllocator);
        }
        vkContext.renderEnd(*frameCxt);
    }

    started = false;
}

IndirectDrawBatch& RenderContext::getStaticBatch(int instanceIdx, const Mesh& mesh, const Material& material) {
    if (staticBatches == 0) {
        auto batchIdx = staticBatches++;

        auto& newDraw = staticBatchCache[batchIdx];
        newDraw.reset();

        newDraw.setMesh(&mesh);
        newDraw.setMaterial(&material);
        newDraw.setCmdId(batchIdx);
        newDraw.setFirstInstanceIdx(instanceIdx);

        if (material.hasShadow())
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

        if (material.hasShadow())
            shadowNonSkinnedBatchCache[staticShadowNonSkinnedBatches++] = newDraw;

        return newDraw;
    }
}

void RenderContext::deferredPass(DescriptorSetAllocator& descSetAllocator) {
    ZoneScoped;

    auto frameIdx = frameCxt->frameIndex;
    auto vkCmd = frameCxt->cmd;

    auto rpCxt = RenderPassContext{ 0, frameIdx, vkCmd, frameCxt->swapchainExtents };

    vkContext.beginRenderPass(rpCxt);
    {
        setViewportScissor(rpCxt.extents);

        // subpass 1
        {
            ZoneScopedN("RenderContext::deferredPass - Record Draw Cmds");

            terrainContext->draw(vkContext, rpCxt, descSetAllocator);

            // move to secondary cmdbuf?
            for (int i = 0; i < staticBatches; i++) {
                auto& batch = staticBatchCache[i];
                batch.draw(vkContext, vkCmd, *indirectCmdBuf,
                    descSetAllocator, frameIdx, bindManager);
            }

            for (int i = 0; i < dynamicBatches; i++) {
                auto& batch = dynamicBatchCache[i];
                batch.draw(vkContext, vkCmd, *indirectCmdBuf,
                    descSetAllocator, frameIdx, bindManager);
            }
        }

        // subpass 2
        compositionSubpass(rpCxt, descSetAllocator);

        // subpass 3 forward transparency pass, TODO
        vkCmdNextSubpass(vkCmd, VK_SUBPASS_CONTENTS_INLINE);

#ifdef KE_DEBUG_RENDER
        //subpass 4 debug
        debugSubpass(rpCxt, descSetAllocator);
#endif

        //subpass 4 or 5 GUI (depends on is debug rendering is enabled)
        //guiManager.subpass(vkContext, rpCxt, frameCxt, descSetAllocator);
        // since guimanager isnt implemented yet we need to push to next subpass manually
        // we'll remove this line once the guimanager is working
        vkCmdNextSubpass(vkCmd, VK_SUBPASS_CONTENTS_INLINE);

        if (imGuiContext)
            imGuiContext->draw(*frameCxt);
    }
    vkContext.endRenderPass(rpCxt);
}


#ifdef KE_DEBUG_RENDER
void RenderContext::debugSubpass(RenderPassContext& rpCxt, DescriptorSetAllocator& descSetAllocator) {
    auto frameIdx = frameCxt->frameIndex;
    auto vkCmd = frameCxt->cmd;

    vkCmdNextSubpass(vkCmd, VK_SUBPASS_CONTENTS_INLINE);

    // submit debug draws

    auto& p1 = vkContext.getPipelineCache().getPipeline<DebugDeferredOffscreenPbrPipeline>();
    p1.bind(vkContext, descSetAllocator, vkCmd, frameIdx);

    auto& indexBuf = debugMesh->getIndexBuf();
    auto indexCount = debugMesh->getIndexCount();

    VkDeviceSize offsets = 0;
    vkCmdBindVertexBuffers(vkCmd, 0, 1, &debugMesh->getVertexBuf().vkBuffer, &offsets);
    vkCmdBindIndexBuffer(vkCmd, indexBuf.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

    // change to instancing
    for (auto i = 0; i < totalDebugObjects; i++)
    {
        vkCmdPushConstants(vkCmd, p1.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DebugObject), &debugObjects[i]);
        vkCmdDrawIndexed(vkCmd, indexCount, 1, 0, 0, 0);
    }
}
#endif

void RenderContext::compositionSubpass(RenderPassContext& rpCxt, DescriptorSetAllocator& d) {
    // do this only when swapchain images change
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
