
#include "KinematicPlayerSystem.hpp"
#include <kengine/input/InputManager.hpp>
#include "PlayerMovementManager.hpp"
#include <kengine/Game.hpp>
#include <kengine/SceneGraph.hpp>
#include <components/Physics.hpp>
#include <components/Components.hpp>
#include <kengine/terrain/TerrainContext.hpp>

void KinematicPlayerSystem::init() {
    sceneGraph = getService<ke::SceneGraph>();
    inputManager = getService<ke::InputManager>();
    terrainContext = getService<ke::TerrainContext>();
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
    if (physicsContext->isPaused())
        return;

    auto& ecs = getEcs();
    
    auto& linVelComp = ecs.get<Component::LinearVelocity>(playerEntity);
    auto& spatials = ecs.get<Component::Spatials>(playerEntity);
    auto spatial = sceneGraph->get(spatials.rootSpatialId);

    auto input = getInput();

    playerMovementManager->stepPlayer(sceneTime->getDelta(), *spatial, linVelComp, input);

    // move all this to movement manager?
    {
        auto& linVel = linVelComp.linearVelocity;
        auto& body = playerCtx->getPlayerPhysicsBody();
        body.SetLinearVelocity(JPH::Vec3(linVel.x, linVel.y, linVel.z));

        // stuff from demo
        {
            JPH::Quat character_up_rotation = JPH::Quat::sEulerAngles(JPH::Vec3(0, 0, 0));
            body.SetUp(character_up_rotation.RotateAxisY());
            body.SetRotation(character_up_rotation);

            // A cheaper way to update the character's ground velocity,
            // the platforms that the character is standing on may have changed velocity
            body.UpdateGroundVelocity();
        }

        // preemptively remove gravity to avoid weird physics resolutions in case player is underground
        {
            auto& nextPos = spatial->getWorldTransform().getPosition();
            auto newY = nextPos.y + -10 * sceneTime->getDelta();
            auto terrainHeight = terrainContext->getTerrain().getHeightAt(nextPos.x, nextPos.z);
            if (newY <= terrainHeight) {
                auto bodyPos = body.GetPosition();
                body.SetPosition(JPH::Vec3(bodyPos.GetX(), terrainHeight, bodyPos.GetZ()));
                linVelComp.linearVelocity.y = 0;
            }
        }

        auto& physics = physicsContext->getPhysics();
        JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;
        body.ExtendedUpdate(sceneTime->getDelta(),
            JPH::Vec3(0, -10, 0),
            update_settings,
            physics.GetDefaultBroadPhaseLayerFilter(Layers::PLAYER),
            physics.GetDefaultLayerFilter(Layers::PLAYER),
            { },
            { },
            physicsContext->getTempAllocator());

        auto bodyPos = body.GetPosition();
        auto newPos = glm::vec3(bodyPos.GetX(), bodyPos.GetY(), bodyPos.GetZ());

        // terrain collision
        {
            auto terrainHeight = terrainContext->getTerrain().getHeightAt(newPos.x, newPos.z);
            if (newPos.y <= terrainHeight) {
                newPos.y = terrainHeight;
                linVelComp.linearVelocity.y = 0;
                body.SetPosition(JPH::Vec3(newPos.x, newPos.y, newPos.z));
            }
        }

        spatial->setLocalPosition(newPos);
    }
}
