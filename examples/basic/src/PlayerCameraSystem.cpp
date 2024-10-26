
#include "PlayerCameraSystem.hpp"
#include <kengine/Logger.hpp>
#include <kengine/ecs/World.hpp>
#include "components/Physics.hpp"
#include "components/Components.hpp"

void PlayerCameraSystem::init() {
    sceneGraph = getService<ke::SceneGraph>();
    cameraController = getService<PlayerCameraController>();
}

void PlayerCameraSystem::processSystem(entt::entity playerEntity) {
    auto spatials = getEcs().get<Component::Spatials>(playerEntity);
}

