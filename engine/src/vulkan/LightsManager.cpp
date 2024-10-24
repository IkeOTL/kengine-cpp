#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>

namespace ke {
    void LightsManager::upload(VulkanContext& vkCxt, CachedGpuBuffer& buffer, float alpha, int frameIndex) {
        struct Light {
            glm::vec4 color;
            glm::vec3 pos;
            float radius;
        };

        struct ToUpload {
            glm::vec3 viewPos;
            uint32_t lightCount;
            glm::mat4 view;
            std::vector<Light> lights;
        };

        auto camera = cameraController.getCamera();
        ToUpload data{}; // todo: should we memcpy here instead?
        data.viewPos = camera->getPosition();
        camera->getViewMatrix(data.view);
        //camera->getIntegratedViewMatrix(alpha, data.view);
        data.lightCount = 0;

        auto startPos = buffer.getFrameOffset(frameIndex);
        memcpy(static_cast<char*>(buffer.getGpuBuffer().data()) + startPos, &data, sizeof(ToUpload));
        buffer.getGpuBuffer().flush(startPos, sizeof(ToUpload));
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
} // namespace ke