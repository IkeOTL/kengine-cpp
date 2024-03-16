#pragma once
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/input/InputEventAdapter.hpp>


class BasicCameraController : public CameraController {
private:

public:
    BasicCameraController() {}

    void update(float delta) override;

    void setPosition(float x, float y, float z);
};

class FreeCameraController : public InputEventAdapter, public CameraController {
private:
    void init();

public:
    FreeCameraController(InputManager& inputManager) : InputEventAdapter(inputManager) {
        init();
    }

    void update(float delta) override;

    void setPosition(float x, float y, float z);
};