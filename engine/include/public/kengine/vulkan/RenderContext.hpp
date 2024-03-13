#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/MaterialBindManager.hpp>
#include <kengine/vulkan/SceneData.hpp>
#include <kengine/vulkan/ShadowContext.hpp>
#include <kengine/vulkan/IndirectDrawBatch.hpp>
#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/vulkan/CullContext.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <mutex>

class CachedGpuBuffer;
class CameraController;

struct ObjectInstance {
    uint32_t cmdId;
    uint32_t instanceIdx;
};

class RenderContext {
public:
    const static uint32_t MAX_INSTANCES = 200000;
    const static uint32_t MAX_BATCHES = 10000;

private:
    VulkanContext& vkContext;
    GpuBufferCache& bufCache;

    std::unique_ptr<CullContext> cullContext;
    std::unique_ptr<ShadowContext> shadowContext;
    CameraController& cameraController;
    LightsManager& lightsManager;

    std::unique_ptr<SceneData> sceneData;

    std::vector<std::unique_ptr<DescriptorSetAllocator>> descSetAllocators;

    // bufs
    CachedGpuBuffer* sceneBuf = nullptr;
    CachedGpuBuffer* materialsBuf = nullptr;
    CachedGpuBuffer* lightBuf = nullptr;

    CachedGpuBuffer* indirectCmdBuf = nullptr;
    CachedGpuBuffer* drawObjectBuf = nullptr;
    CachedGpuBuffer* objectInstanceBuf = nullptr; // all instances submitted before GPU culling
    CachedGpuBuffer* drawInstanceBuffer = nullptr; // all instances after GPU culling

    uint32_t staticInstances = 0;
    uint32_t staticBatches = 0;
    uint32_t staticShadowNonSkinnedBatches = 0;
    IndirectDrawBatch staticBatchCache[MAX_INSTANCES];

    uint32_t dynamicInstances = 0;
    uint32_t dynamicBatches = 0;
    IndirectDrawBatch dynamicBatchCache[MAX_INSTANCES];

    uint32_t totalShadowSkinnedBatches = 0;
    uint32_t totalShadowNonSkinnedBatches = 0;
    IndirectDrawBatch shadowSkinnedBatchCache[MAX_INSTANCES / 4];
    IndirectDrawBatch shadowNonSkinnedBatchCache[MAX_INSTANCES / 4];

    MaterialBindManager bindManager;

    RenderFrameContext* frameCxt = nullptr;

    float alpha = 0;
    float sceneTime = 0;

    boolean started = false;

    void setViewportScissor(glm::uvec2 dim);
    void initBuffers();
    void initDescriptors();

    IndirectDrawBatch& getStaticBatch(int instanceIdx, const Mesh& mesh, const Material& material, boolean hasShadow);
    void deferredPass(DescriptorSetAllocator& descSetAllocator);
    void compositionSubpass(RenderPassContext& rpCxt, DescriptorSetAllocator& d);

public:
    RenderContext(VulkanContext& vkCtx, GpuBufferCache& bufCache,
        LightsManager& lightsManager, CameraController& cameraController)
        : vkContext(vkCtx), bufCache(bufCache), lightsManager(lightsManager),
        cameraController(cameraController), bindManager(MaterialBindManager(vkCtx)) {}

    void init();

    void addStaticInstance(const Mesh& mesh, const Material& material, glm::mat4 transform, glm::vec4 boundingSphere, boolean hasShadow);
    int draw(const Mesh& mesh, const Material& material, glm::mat4 transform, glm::vec4 boundingSphere);
    void begin(RenderFrameContext& frameCxt, float sceneTime, float alpha);
    void end();

    const std::vector<std::unique_ptr<DescriptorSetAllocator>>& getDescSetAllocators() const {
        return descSetAllocators;
    }

    DescriptorSetAllocator& getDescSetAllocator(int i) const {
        return *descSetAllocators[i];
    }
};