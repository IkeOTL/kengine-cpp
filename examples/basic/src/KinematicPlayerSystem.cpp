
#include "KinematicPlayerSystem.hpp"
#include <kengine/input/InputManager.hpp>
#include "PlayerMovementManager.hpp"
#include <kengine/Game.hpp>
#include <kengine/SceneGraph.hpp>
#include <components/Physics.hpp>
#include <components/Components.hpp>

void KinematicPlayerSystem::init() {
    sceneGraph = getService<ke::SceneGraph>();
    inputManager = getService<ke::InputManager>();
    playerMovementManager = getService<PlayerMovementManager>();
    sceneTime = getService<ke::SceneTime>();
    physicsContext = getService<PhysicsContext>();
}

int KinematicPlayerSystem::getInput() {
    return (inputManager->isKeyDown(GLFW_KEY_W) ? 1 : 0)
        | (inputManager->isKeyDown(GLFW_KEY_S) ? 1 << 1 : 0)
        | (inputManager->isKeyDown(GLFW_KEY_A) ? 1 << 2 : 0)
        | (inputManager->isKeyDown(GLFW_KEY_D) ? 1 << 3 : 0);
}

void KinematicPlayerSystem::processSystem(entt::entity playerEntity) {
    auto& ecs = getEcs();

    auto& linVelComp = ecs.get<Component::LinearVelocity>(playerEntity);
    auto& spatials = ecs.get<Component::Spatials>(playerEntity);
    auto spatial = sceneGraph->get(spatials.rootSpatialId);

    auto input = getInput();

    playerMovementManager->stepPlayer(sceneTime->getDelta(), *spatial, linVelComp, input);

    const auto& linVel = linVelComp.linearVelocity;
    auto& body = playerCtx->getPlayerPhysicsBody();
    body.SetLinearVelocity(JPH::Vec3(linVel.x, linVel.y, linVel.z));

    auto& physics = physicsContext->getPhysics();
    JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;
    body.ExtendedUpdate(sceneTime->getDelta(),
        JPH::Vec3(0, -10, 0),
        update_settings,
        physics.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
        physics.GetDefaultLayerFilter(Layers::MOVING),
        { },
        { },
        physicsContext->getTempAllocator());

    int i = 0;
}
