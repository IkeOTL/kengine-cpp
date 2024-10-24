#pragma once
#include <kengine/ecs/EcsSystem.hpp>
#include <thirdparty/entt.hpp>
#include <memory>

namespace ke {
    class CameraController;
}

class CameraSystem : public ke::EcsSystem {
private:
    ke::CameraController* cameraController;

public:
    CameraSystem() = default;

    bool checkProcessing() override {
        return true;
    }

    void init() override;
    void processSystem(float delta) override;
};