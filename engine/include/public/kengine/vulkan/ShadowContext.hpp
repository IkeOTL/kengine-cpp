#pragma once
#include <kengine/vulkan/ShadowCascade.hpp>
#include <glm/vec3.hpp>
#include <vector>

class IndirectDrawBatch;
class VulkanContext;
class DescriptorSetAllocator;
class CameraController;
class SceneData;
class RenderFrameContext;
class Pipeline;

class ShadowContext {
private:
    GpuBufferCache& bufCache;
    CachedGpuBuffer& indirectCmdBuf;
    CameraController& cameraController;
    SceneData& sceneData;

    ShadowCascadeData cascadesData{};

    CachedGpuBuffer* shadowPassCascadeBuf = nullptr;
    CachedGpuBuffer* compositePassCascadeBuf = nullptr;

public:
    ShadowContext(GpuBufferCache& bufCache, CachedGpuBuffer& indirectCmdBuf, CameraController& cameraController, SceneData& sceneData)
        :bufCache(bufCache), indirectCmdBuf(indirectCmdBuf), cameraController(cameraController), sceneData(sceneData) {}

    void init(VulkanContext& vkContext, std::vector<DescriptorSetAllocator>& descSetAllocators,
        glm::vec3 lightDir, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer& drawInstanceBuffer);

    void execute(VulkanContext& vkContext, RenderFrameContext& cxt, DescriptorSetAllocator& dAllocator,
        IndirectDrawBatch* nonSkinnedBatches, size_t nonSkinnedBatchesSize,
        IndirectDrawBatch* skinnedBatches, size_t skinnedBatchesSize);

    void execShadowPass(VulkanContext& vkContext, RenderFrameContext& cxt, Pipeline& p1,
        DescriptorSetAllocator& dAllocator, size_t cascadeIdx,
        IndirectDrawBatch* batches, size_t batchesSize, bool skinned);

};