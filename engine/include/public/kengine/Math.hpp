#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

namespace math {
    enum FrustumPlane {
        PLANE_NX,
        PLANE_PX,
        PLANE_NY,
        PLANE_PY,
        PLANE_NZ,
        PLANE_PZ
    };

    static glm::vec4 frustumPlane(const glm::mat4& matrix, FrustumPlane plane) {
        glm::vec4 dest;
        switch (plane) {
        case PLANE_NX:
            dest = glm::vec4(matrix[0][3] + matrix[0][0], matrix[1][3] + matrix[1][0], matrix[2][3] + matrix[2][0], matrix[3][3] + matrix[3][0]);
            break;
        case PLANE_PX:
            dest = glm::vec4(matrix[0][3] - matrix[0][0], matrix[1][3] - matrix[1][0], matrix[2][3] - matrix[2][0], matrix[3][3] - matrix[3][0]);
            break;
        case PLANE_NY:
            dest = glm::vec4(matrix[0][3] + matrix[0][1], matrix[1][3] + matrix[1][1], matrix[2][3] + matrix[2][1], matrix[3][3] + matrix[3][1]);
            break;
        case PLANE_PY:
            dest = glm::vec4(matrix[0][3] - matrix[0][1], matrix[1][3] - matrix[1][1], matrix[2][3] - matrix[2][1], matrix[3][3] - matrix[3][1]);
            break;
        case PLANE_NZ:
            dest = glm::vec4(matrix[0][3] + matrix[0][2], matrix[1][3] + matrix[1][2], matrix[2][3] + matrix[2][2], matrix[3][3] + matrix[3][2]);
            break;
        case PLANE_PZ:
            dest = glm::vec4(matrix[0][3] - matrix[0][2], matrix[1][3] - matrix[1][2], matrix[2][3] - matrix[2][2], matrix[3][3] - matrix[3][2]);
            break;
        default:
            throw std::runtime_error("Invalid plane index");
        }

        // normalize using only XYZ
        auto invLength = 1 / std::sqrt(std::fma(dest.x, dest.x, std::fma(dest.y, dest.y, dest.z * dest.z)));
        return dest *= invLength;
    }

    static glm::vec3 transformProject(const glm::mat4& matrix, glm::vec3& vec) {
        glm::vec4 transformedVec = matrix * glm::vec4(vec, 1.0f);
        auto invW = 1 / transformedVec.w; // for perspective division
        return glm::vec3(transformedVec) * invW;
    }
}