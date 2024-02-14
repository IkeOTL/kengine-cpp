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

        var objInstanceStartPos = (int)objectInstanceBuf.getFrameOffset(frameIdx);
        var curObjInstancePos = objInstanceStartPos + (instanceIdx * (2 * Integer.BYTES));

        // buf upload            
        objectInstanceBuf.getVmaBuffer().buf()
            .position(curObjInstancePos)
            .putInt(draw.getCmdId())
            .putInt(instanceIdx);
    }
}

int RenderContext::draw(Mesh& mesh, Material& material, glm::mat4 transform, glm::vec4 boundingSphere, boolean hasShadow, boolean hasSkeleton) {

}

void RenderContext::begin(RenderFrameContext& frameCxt, float sceneTime, float alpha) {

}

void RenderContext::end() {

}

void RenderContext::setViewportScissor(glm::uvec2 dim) {

}

void RenderContext::initBuffers() {

}

void RenderContext::initDescriptors() {

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

}

void RenderContext::compositionSubpass(RenderPassContext& rpCxt, DescriptorSetAllocator& d) {

}
