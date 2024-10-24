#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include "PhysicsContext.hpp"

class PhysicsSystem : public ke::BaseSystem {
private:
    PhysicsContext* physicsContext;

    bool paused = false;

public:
    void initialize() override;
    void processSystem(float delta) override;

    void setPaused(bool b) {
        paused = b;
    }

    bool isPaused() const {
        return paused;
    }
};