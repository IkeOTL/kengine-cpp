#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <bit>

namespace math {

    inline static float toRadians(float degrees) {
        static constexpr float toRads = 3.14159265358979323846f / 180.0f;
        return degrees * toRads;
    }

    inline static float invsqrt(float r) {
        return 1.0f / std::sqrtf(r);
    }


}