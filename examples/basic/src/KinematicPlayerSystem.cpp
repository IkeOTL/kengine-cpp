
#include "KinematicPlayerSystem.hpp"
#include <kengine/input/InputManager.hpp>
#include "PlayerMovementManager.hpp"
#include <kengine/Game.hpp>
#include <kengine/SceneGraph.hpp>

void KinematicPlayerSystem::init() {
    sceneGraph = getService<SceneGraph>();
    inputManager = getService<InputManager>();
    playerMovementManager = getService<PlayerMovementManager>();
    sceneTime = getService<SceneTime>();
}


int KinematicPlayerSystem::getInput() {
    return (inputManager->isKeyDown(GLFW_KEY_W) ? 1 : 0)
        | (inputManager->isKeyDown(GLFW_KEY_S) ? 1 << 1 : 0)
        | (inputManager->isKeyDown(GLFW_KEY_A) ? 1 << 2 : 0)
        | (inputManager->isKeyDown(GLFW_KEY_D) ? 1 << 3 : 0);
}

void KinematicPlayerSystem::processSystem(entt::entity playerEntity)
{
    auto& ecs = getEcs();

    auto& vel = ecs.get<Component::LinearVelocity>(playerEntity);
    auto& spatials = ecs.get<Component::Spatials>(playerEntity);
    auto spatial = sceneGraph->get(spatials.rootSpatialId);

    auto input = getInput();

    playerMovementManager->stepPlayer(sceneTime->getDelta(), *spatial, vel, input);
}
