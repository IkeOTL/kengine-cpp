#include <kengine/Bounds.hpp>
#include <kengine/util/MatUtils.hpp>

namespace ke {
    // aabb
    void Aabb::getMinMax(glm::vec3& min, glm::vec3& max) const {
        min = pos - extents;
        max = pos + extents;
    }

    Aabb Aabb::transform(const glm::mat4& mat) {
        glm::vec3 min{};
        glm::vec3 max{};
        getMinMax(min, max);
        matutils::transformAab(mat, min, max, min, max);

        return Aabb::fromMinMax(min, max);
    }

    Aabb Aabb::fromMinMax(const glm::vec3& min, const glm::vec3& max) {
        return Aabb((max + min) * 0.5f, (max - min) * 0.5f);
    }

    // bounds
    Bounds::Bounds(const glm::vec3& pos, const glm::vec3& extents) :
        aabb(pos, extents),
        sphereBounds(pos, glm::length(extents)) {}

    Bounds::Bounds(const Aabb& aabb) :
        Bounds(aabb.pos, aabb.extents) {}

    Bounds Bounds::transform(const glm::mat4& mat) {
        return Bounds(aabb.transform(mat));
    }

    Bounds Bounds::fromMinMax(const glm::vec3& min, const glm::vec3& max) {
        return Bounds(Aabb::fromMinMax(min, max));
    }
} // namespace ke
