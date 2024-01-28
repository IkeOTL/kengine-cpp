#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/CullContext.hpp>
#include <kengine/vulkan/ShadowContext.hpp>

#include <mutex>

class RenderContext {

private:
    VulkanContext& vkContext;
    GpuBufferCache& bufCache;

    std::unique_ptr<CullContext> cullContext;
    std::unique_ptr<ShadowContext> shadowContext;
    CameraController& cameraController;

    // bufs
    CachedGpuBuffer& sceneBuf;
    CachedGpuBuffer& materialsBuf;
    CachedGpuBuffer& lightBuf;

    CachedGpuBuffer& indirectCmdBuf;
    CachedGpuBuffer& drawObjectBuf;
    CachedGpuBuffer& objectInstanceBuf; // all instances submitted before GPU culling
    CachedGpuBuffer& drawInstanceBuffer; // all instances after GPU culling

public:
    const static uint32_t MAX_INSTANCES = 200000;
    const static uint32_t MAX_BATCHES = 10000;


    void init();
};