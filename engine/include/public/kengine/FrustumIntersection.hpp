#pragma once
#include <glm/glm.hpp>
#include <array>

namespace ke {
    /// <summary>
    /// Ported from the Java library JOML
    /// </summary>
    class FrustumIntersection {
    private:
        float nxX, nxY, nxZ, nxW;
        float pxX, pxY, pxZ, pxW;
        float nyX, nyY, nyZ, nyW;
        float pyX, pyY, pyZ, pyW;
        float nzX, nzY, nzZ, nzW;
        float pzX, pzY, pzZ, pzW;

        std::array<glm::vec4, 6> planes{};

    public:
        void set(const glm::mat4& m, bool allowTestSpheres);
        bool testAab(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) const;
    };
} // namespace ke