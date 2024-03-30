#pragma once
#include <glm/glm.hpp>
#include <bit>

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
}