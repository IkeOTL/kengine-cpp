
#include "PlayerCameraSystem.hpp"
#include <kengine/Logger.hpp>
#include <kengine/ecs/World.hpp>
#include "components/Physics.hpp"
#include "components/Components.hpp"
#include <glm/glm.hpp>
#include <kengine/Game.hpp>

void PlayerCameraSystem::init() {
    sceneTime = getService<ke::SceneTime>();
    sceneGraph = getService<ke::SceneGraph>();
    cameraController = static_cast<PlayerCameraController*>(getService<ke::CameraController>());
}

void PlayerCameraSystem::processSystem(entt::entity playerEntity) {
    auto& spatials = getEcs().get<Component::Spatials>(playerEntity);

    auto spatial = sceneGraph->get(spatials.rootSpatialId);

    auto& cameraPos = cameraController->getCamera()->getPosition();
    auto desiredPosition = spatial->getPosition() + glm::vec3(5, 7, 5);

    auto posDiff = desiredPosition - cameraPos;
    auto distanceSq = glm::dot(posDiff, posDiff);
    auto speed = glm::clamp(distanceSq * 0.5f, 0.5f, 20.0f);

    desiredPosition = glm::mix(cameraPos, desiredPosition, speed * sceneTime->getDelta());

    cameraController->setPosition(desiredPosition.x, desiredPosition.y, desiredPosition.z);
}
