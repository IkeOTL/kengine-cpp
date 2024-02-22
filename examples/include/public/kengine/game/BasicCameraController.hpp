#pragma once
#include <kengine/vulkan/CameraController.hpp>


class BasicCameraController : public CameraController {
private:

public:
    BasicCameraController() {}

    void update(float delta) override;

    void setPosition(float x, float y, float z);
};