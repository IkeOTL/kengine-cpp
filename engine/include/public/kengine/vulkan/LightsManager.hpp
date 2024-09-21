#pragma once
#include <kengine/vulkan/CameraController.hpp>
#include <memory>
#include <functional>

class VulkanContext;
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

    inline static std::unique_ptr<LightsManager> create(CameraController& cameraController) {
        return std::make_unique<LightsManager>(cameraController);
    }

    void upload(VulkanContext& vkCxt, CachedGpuBuffer& buffer, float alpha, int frameIndex);

    static int alignedFrameSize(VulkanContext& vkCxt);
    static int size();
};
