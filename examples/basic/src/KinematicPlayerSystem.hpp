#pragma once
#include "MyPlayerSystem.hpp"

class SceneGraph;
class InputManager;
class PlayerMovementManager;
class SceneTime;

class KinematicPlayerSystem : public MyPlayerSystem {
private:
    SceneGraph* sceneGraph;
    InputManager* inputManager;
    PlayerMovementManager* playerMovementManager;
    SceneTime* sceneTime;

protected:
    int getInput();

public:
    void init() override;
    void processSystem(entt::entity playerEntity) override;
};