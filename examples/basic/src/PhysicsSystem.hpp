#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include "PhysicsContext.hpp"

class PhysicsSystem : public ke::BaseSystem {
private:
    PhysicsContext* physicsContext;

public:
    void initialize() override;
    void processSystem(float delta) override;
};