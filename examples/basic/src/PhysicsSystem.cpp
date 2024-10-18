#include "PhysicsSystem.hpp"
#include <kengine/ecs/World.hpp>

void PhysicsSystem::initialize() {
    physicsContext = getWorld().getService<PhysicsContext>();
}

void PhysicsSystem::processSystem(float delta) {
    physicsContext->step(delta);
}