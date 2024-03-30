#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <bit>

namespace math {

    inline static float min(float f0, float f1) {
        return f0 < f1 ? f0 : f1;
    }

    inline static float max(float f0, float f1) {
        return f0 > f1 ? f0 : f1;
    }

    inline static float toRadians(float degrees) {
        static constexpr float toRads = 3.14159265358979323846f / 180.0f;
        return degrees * toRads;
    }

    inline static float invsqrt(float r) {
        return 1.0f / std::sqrtf(r);
    }


}