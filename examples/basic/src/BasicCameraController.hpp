#pragma once
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/input/InputEventAdapter.hpp>
#include <kengine/FrustumIntersection.hpp>
#include <glm/glm.hpp>
#include <array>

class BasicCameraController : public CameraController {
private:

public:
    BasicCameraController() {}

    void update(float delta) override;

    void setPosition(float x, float y, float z);
};

class FreeCameraController : public InputEventAdapter, public CameraController {
private:
    // move to abstract class
    FrustumIntersection frustumTester{};
    glm::vec3 frustumMin{}, frustumMax{};
    std::array<glm::vec3, 8> frustumCorners{};
    bool needsFrustumRecalc = true;

    void init();

public:
    FreeCameraController(InputManager& inputManager);

    void update(float delta) override;

    void applyMovement(float delta);

    void setPosition(float x, float y, float z);

    std::array<glm::vec3, 8>& getFrustumCorners() {
        return frustumCorners;
    }

    FrustumIntersection& getFrustumTester() {
        return frustumTester;
    }
};