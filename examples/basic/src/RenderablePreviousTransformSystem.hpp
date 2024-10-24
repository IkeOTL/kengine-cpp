#pragma once
#include <kengine/ecs/EcsSystem.hpp>

namespace ke {
    class World;
    class CameraController;
    class SceneGraph;
    class Transform;
    class ExecutorService;
}

class RenderablePreviousTransformSystem : public ke::EcsSystem {
private:
    ke::ExecutorService* executorService;
    ke::SceneGraph* sceneGraph;
    ke::CameraController* cameraController;

public:
    RenderablePreviousTransformSystem() = default;

    bool checkProcessing() override {
        return false;
    }

    void init() override;
    void processSystem(float delta) override;
};