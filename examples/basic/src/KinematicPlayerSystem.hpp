#pragma once
#include "MyPlayerSystem.hpp"

namespace ke {
    class SceneGraph;
    class InputManager;
    class SceneTime;
}

class PlayerMovementManager;

class KinematicPlayerSystem : public MyPlayerSystem {
private:
    ke::SceneGraph* sceneGraph;
    ke::InputManager* inputManager;
    PlayerMovementManager* playerMovementManager;
    ke::SceneTime* sceneTime;

protected:
    int getInput();

public:
    void init() override;
    void processSystem(entt::entity playerEntity) override;
};