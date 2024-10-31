#pragma once
#include <kengine/Spatial.hpp>
#include <components/Physics.hpp>
#include <memory>

class PlayerMovementManager {
private:
    inline static const float GRAVITY = -10;

    void calculateLinearVelocity(float step, uint32_t input, glm::vec3& currentVel);

public:
    inline static std::unique_ptr<PlayerMovementManager> create() {
        return std::make_unique<PlayerMovementManager>();
    }

    void stepPlayer(float step, ke::Spatial& spatial, Component::LinearVelocity& velComp, uint32_t input);
};