#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/ShadowCascade.hpp>
#include <kengine/vulkan/SceneData.hpp>
#include <glm/vec3.hpp>
#include <vector>

class IndirectDrawBatch;

class ShadowContext {
private:
    GpuBufferCache& bufCache;
    CachedGpuBuffer& indirectCmdBuf;
    CameraController& cameraController;
    SceneData& sceneData;

    ShadowCascadeData cascadesData{};

    CachedGpuBuffer* shadowPassCascadeBuf;
    CachedGpuBuffer* compositePassCascadeBuf;

public:
    void init(VulkanContext& vkContext, std::vector<DescriptorSetAllocator>& descSetAllocators,
        glm::vec3 lightDir, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer drawInstanceBuffer);

    void execute(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt, DescriptorSetAllocator& dAllocator,
        IndirectDrawBatch* nonSkinnedBatches, size_t nonSkinnedBatchesSize,
        IndirectDrawBatch* skinnedBatches, size_t skinnedBatchesSize);

    void execShadowPass(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt, Pipeline& p1,
        DescriptorSetAllocator& dAllocator, size_t cascadeIdx,
        IndirectDrawBatch* batches, size_t batchesSize, bool skinned);

};