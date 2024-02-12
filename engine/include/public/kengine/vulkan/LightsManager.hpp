#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <functional>

class VulkanContext;
class CameraController;
class CachedGpuBuffer;

class Light {

};

class LightsManager {
private:
    CameraController& cameraController;
    std::vector<std::unique_ptr<Light>> lights;

public:
    inline static const size_t MAX_LIGHTS = 64;

    LightsManager(CameraController& cameraController) : cameraController(cameraController) {}

    void upload(VulkanContext& vkCxt, CachedGpuBuffer& buffer, int frameIndex);

    static int alignedFrameSize(VulkanContext& vkCxt);
    static int size();
};
