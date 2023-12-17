#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <mutex>

class VulkanContext;
class GpuBufferCache;
class CullContext;

class RenderContext {

private:
    VulkanContext& vkContext;
    GpuBufferCache& bufCache;

    std::unique_ptr<CullContext> cullContext;

public:
    const static size_t MAX_INSTANCES = 200000;
};