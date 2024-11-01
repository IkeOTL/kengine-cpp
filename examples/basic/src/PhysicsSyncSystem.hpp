#pragma once
#include <kengine/ecs/EcsSystem.hpp>
#include <kengine/SceneGraph.hpp>
#include "PhysicsContext.hpp"

class PhysicsSyncSystem : public ke::EcsSystem {
private:
    PhysicsContext* physicsContext;
    ke::SceneGraph* sceneGraph;

public:
    void init() override;
    void processSystem(float delta) override;
};