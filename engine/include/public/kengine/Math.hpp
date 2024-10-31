#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <bit>

namespace ke {
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

        inline void setFromNormalized(glm::quat& dest, float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22) {
            float t;
            float tr = m00 + m11 + m22;
            if (tr >= 0.0f) {
                t = std::sqrt(tr + 1.0f);
                dest.w = t * 0.5f;
                t = 0.5f / t;
                dest.x = (m12 - m21) * t;
                dest.y = (m20 - m02) * t;
                dest.z = (m01 - m10) * t;
            } else {
                if (m00 >= m11 && m00 >= m22) {
                    t = std::sqrt(m00 - (m11 + m22) + 1.0f);
                    dest.x = t * 0.5f;
                    t = 0.5f / t;
                    dest.y = (m10 + m01) * t;
                    dest.z = (m02 + m20) * t;
                    dest.w = (m12 - m21) * t;
                } else if (m11 > m22) {
                    t = std::sqrt(m11 - (m22 + m00) + 1.0f);
                    dest.y = t * 0.5f;
                    t = 0.5f / t;
                    dest.z = (m21 + m12) * t;
                    dest.x = (m10 + m01) * t;
                    dest.w = (m20 - m02) * t;
                } else {
                    t = std::sqrt(m22 - (m00 + m11) + 1.0f);
                    dest.z = t * 0.5f;
                    t = 0.5f / t;
                    dest.x = (m02 + m20) * t;
                    dest.y = (m21 + m12) * t;
                    dest.w = (m01 - m10) * t;
                }
            }
        }

        inline void getUnnormalizedRotation(glm::mat4& m, glm::quat& dest) {
            float nm00 = m[0][0], nm01 = m[0][1], nm02 = m[0][2];
            float nm10 = m[1][0], nm11 = m[1][1], nm12 = m[1][2];
            float nm20 = m[2][0], nm21 = m[2][1], nm22 = m[2][2];
            float lenX = invsqrt(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]);
            float lenY = invsqrt(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2]);
            float lenZ = invsqrt(m[2][0] * m[2][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2]);
            nm00 *= lenX;
            nm01 *= lenX;
            nm02 *= lenX;
            nm10 *= lenY;
            nm11 *= lenY;
            nm12 *= lenY;
            nm20 *= lenZ;
            nm21 *= lenZ;
            nm22 *= lenZ;
            setFromNormalized(dest, nm00, nm01, nm02, nm10, nm11, nm12, nm20, nm21, nm22);
        }
    } // namespace math
} // namespace ke