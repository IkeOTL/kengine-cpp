#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>

void LightsManager::upload(VulkanContext& vkCxt, CachedGpuBuffer& buffer, int frameIndex) {
    // do nothing for now
}

int LightsManager::alignedFrameSize(VulkanContext& vkCxt) {
    return (int)vkCxt.alignUboFrame(size());
}

int LightsManager::size() {
    return sizeof(glm::vec3)
        + sizeof(int32_t)
        + sizeof(glm::mat4)
        + sizeof(Light) * MAX_LIGHTS;
}