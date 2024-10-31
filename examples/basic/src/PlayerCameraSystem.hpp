#pragma once
#include <kengine/ecs/EcsSystem.hpp>
#include <kengine/SceneGraph.hpp>
#include "PhysicsContext.hpp"
#include "MyPlayerSystem.hpp"
#include "BasicCameraController.hpp"

namespace ke {
    class SceneTime;
}

class PlayerCameraSystem : public MyPlayerSystem {
private:
    ke::SceneTime* sceneTime;
    ke::SceneGraph* sceneGraph;
    PlayerCameraController* cameraController;

public:
    void init() override;
    void processSystem(entt::entity playerEntity) override;
};