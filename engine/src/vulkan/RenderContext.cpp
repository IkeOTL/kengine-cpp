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
    lol
}

void RenderContext::setViewportScissor(glm::uvec2 dim) {
lol
}

void RenderContext::initBuffers() {
    lol
}

void RenderContext::initDescriptors() {
lol
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
    lol
}

void RenderContext::compositionSubpass(RenderPassContext& rpCxt, DescriptorSetAllocator& d) {
    lol
}
