#pragma once
#include <jolt/Jolt.h>
#include <jolt/Physics/Body/BodyID.h>
#include <glm/glm.hpp>

#include <memory>

namespace Component {

    struct Rigidbody {
        JPH::BodyID bodyId;
        bool syncEnabled = true;
    };

    struct LinearVelocity {
        glm::vec3 linearVelocity{0};
    };

    struct TerrainGrounded {
        bool grounded = false;
    };
} // namespace Component