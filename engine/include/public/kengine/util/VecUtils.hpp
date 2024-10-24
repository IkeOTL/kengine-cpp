#pragma once
#include <glm/glm.hpp>
#include <bit>

namespace ke {
    namespace vecutils {
        inline static size_t hashCode(glm::vec4 v) {
            const size_t prime = 31;
            size_t result = 1;
            result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.w));
            result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.x));
            result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.y));
            result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.z));
            return result;
        }

        inline static bool isFinite(const glm::vec3& v) {
            return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
        }

        inline static glm::vec3 min(const glm::vec3& v0, const glm::vec3& v1) {
            glm::vec3 dest;
            dest.x = v0.x < v1.x ? v0.x : v1.x;
            dest.y = v0.y < v1.y ? v0.y : v1.y;
            dest.z = v0.z < v1.z ? v0.z : v1.z;
            return dest;
        }

        inline static glm::vec3 max(const glm::vec3& v0, const glm::vec3& v1) {
            return glm::vec3{
                v0.x > v1.x ? v0.x : v1.x,
                v0.y > v1.y ? v0.y : v1.y,
                v0.z > v1.z ? v0.z : v1.z,
            };
        }
    }
} // namespace ke