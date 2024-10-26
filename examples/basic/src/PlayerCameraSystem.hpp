#pragma once
#include <kengine/ecs/EcsSystem.hpp>
#include <kengine/SceneGraph.hpp>
#include "PhysicsContext.hpp"
#include "MyPlayerSystem.hpp"
#include "BasicCameraController.hpp"

class PlayerCameraSystem : public MyPlayerSystem {
private:
    ke::SceneGraph* sceneGraph;
    PlayerCameraController* cameraController;

public:
    void init() override;
    void processSystem(entt::entity playerEntity) override;
};