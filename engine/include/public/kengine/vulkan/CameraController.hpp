#pragma once
#include <kengine/vulkan/Camera.hpp>
#include <memory>

class CameraController {
protected:
    std::unique_ptr<Camera> camera;

public:
    virtual void update(float delta) = 0;

    Camera* getCamera() {
        return camera.get();
    }

    void setCamera(std::unique_ptr<Camera>&& camera) {
        this->camera = std::move(camera);
    }
};