#pragma once
#include <kengine/ecs/EcsSystem.hpp>

class World;
class CameraController;
class SceneGraph;
class Transform;
class ExecutorService;

class RenderablePreviousTransformSystem : public EcsSystem {
private:
    ExecutorService* executorService;
    SceneGraph* sceneGraph;
    CameraController* cameraController;

public:
    RenderablePreviousTransformSystem() = default;

    bool checkProcessing() override {
        return false;
    }

    void init() override;
    void processSystem(float delta) override;
};