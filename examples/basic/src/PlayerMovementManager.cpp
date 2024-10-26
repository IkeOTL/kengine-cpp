
#include "PlayerMovementManager.hpp"
#include <kengine/util/VecUtils.hpp>
#include <components/Physics.hpp>


void PlayerMovementManager::stepPlayer(float step, ke::Spatial& spatial, Component::LinearVelocity& velComp, uint32_t input) {
    calculateLinearVelocity(step, input, velComp.linearVelocity);
}


void PlayerMovementManager::calculateLinearVelocity(float step, uint32_t input, glm::vec3& currentVel) {
    glm::vec3 targetDir{ 0 };

    if (input & 1) {
        targetDir.x += -1;
        targetDir.z += -1;
    }

    if (input & (1 << 1)) {
        targetDir.x += 1;
        targetDir.z += 1;
    }

    if (input & (1 << 2)) {
        targetDir.x += -1;
        targetDir.z += 1;
    }

    if (input & (1 << 3)) {
        targetDir.x += 1;
        targetDir.z += -1;
    }

    if (glm::dot(targetDir, targetDir) > 0)
        targetDir = glm::normalize(targetDir);

    if (!ke::vecutils::isFinite(targetDir))
        targetDir = glm::vec3{ 0 };

    auto gravity = currentVel.y + GRAVITY * step;
    currentVel = 2.75f * targetDir;

    currentVel.y = gravity;
    //currentVel.y = 0;
}