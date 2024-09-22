#pragma once
#include <kengine/ecs/EcsSystem.hpp>
#include <thirdparty/entt.hpp>
#include <memory>

class CameraController;

class CameraSystem : public EcsSystem {
private:
    CameraController* cameraController;

public:
    CameraSystem() = default;

    bool checkProcessing() override {
        return true;
    }

    void init() override;
    void processSystem(float delta) override;
};