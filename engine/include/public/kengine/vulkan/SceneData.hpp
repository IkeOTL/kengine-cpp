#pragma once
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ke {
    class SceneData {

    private:
        CameraController& cameraController;
        glm::vec3 lightDir;

    public:
        SceneData(CameraController& cameraController, glm::vec3 lightDir)
            : cameraController(cameraController), lightDir(lightDir) {}

        static size_t alignedFrameSize(VulkanContext& vkCxt);
        static size_t size();

        glm::vec3& getLightDir() {
            return lightDir;
        }

        void upload(VulkanContext& vkCxt, CachedGpuBuffer& buffer, float time, float alpha, size_t frameIndex);
    };
} // namespace ke