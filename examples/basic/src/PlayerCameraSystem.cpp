
#include "PlayerCameraSystem.hpp"
#include <kengine/Logger.hpp>
#include <kengine/ecs/World.hpp>
#include "components/Physics.hpp"
#include "components/Components.hpp"

void PlayerCameraSystem::init() {
    sceneGraph = getService<ke::SceneGraph>();
    cameraController = static_cast<PlayerCameraController*>(getService<ke::CameraController>());
}

void PlayerCameraSystem::processSystem(entt::entity playerEntity) {
    auto& spatials = getEcs().get<Component::Spatials>(playerEntity);

    auto spatial = sceneGraph->get(spatials.rootSpatialId);
    auto& pos = spatial->getPosition();
    cameraController->setPosition(pos.x + 5, pos.y + 7, pos.z + 5);

}

