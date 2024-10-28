
#include "PhysicsSyncSystem.hpp"
#include <kengine/Logger.hpp>
#include <kengine/ecs/World.hpp>
#include "components/Physics.hpp"
#include "components/Components.hpp"

void PhysicsSyncSystem::init() {
    physicsContext = getService<PhysicsContext>();
    sceneGraph = getService<ke::SceneGraph>();
}

// merge into PhysicSystem?
void PhysicsSyncSystem::processSystem(float delta) {
    if (physicsContext->isPaused())
        return;

    auto& physics = physicsContext->getPhysics();
    auto& ecs = getEcs();

    auto view = ecs.view<Component::Rigidbody, Component::Spatials>();

    auto& bodyInterface = physics.GetBodyInterfaceNoLock();

    // parallelize this eventually?
    JPH::Vec3 pos;
    JPH::Quat rot;
    for (auto entity : view) {
        auto& rbC = view.get<Component::Rigidbody>(entity);

        if (!rbC.syncEnabled)
            continue;

        auto& sc = view.get<Component::Spatials>(entity);

        auto spatial = sceneGraph->get(sc.rootSpatialId);
        if (!spatial) {
            KE_LOG_WARN(std::format("Spatial not found during physics sync: {}", sc.rootSpatialId));
            continue;
        }

        if (!bodyInterface.IsAdded(rbC.bodyId)) {
            KE_LOG_WARN("BodyID not found in physics system");
            continue;
        }

        // maybe??
        //if (!bodyInterface.IsActive(rbC.bodyId))
        //    continue;

        bodyInterface.GetPositionAndRotation(rbC.bodyId, pos, rot);
        spatial->setLocalPosition(glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
        spatial->setLocalRotation(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
    }
}