#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct Aabb {
public:
    glm::vec3 pos{};
    glm::vec3 extents{};

    Aabb() {};
    Aabb(const glm::vec3& position, const glm::vec3& size)
        : pos(position), extents(size) {}

    Aabb(const Aabb& other) : pos(other.pos), extents(other.extents) { }

    Aabb& operator=(const Aabb& other) {
        if (this != &other) { // protect against invalid self-assignment
            pos = other.pos;
            extents = other.extents;
        }

        return *this;
    }

    void getMinMax(glm::vec3& min, glm::vec3& max);
    Aabb transform(const glm::mat4& mat);

    static Aabb fromMinMax(const glm::vec3& min, const glm::vec3& max);
};

class Bounds {
private:
    Aabb aabb{};
    glm::vec4 sphereBounds{};

public:
    Bounds() {};
    Bounds(const glm::vec3& pos, const glm::vec3& extents);
    Bounds(const Aabb& aabb);

    Bounds(const Bounds& other) : aabb(other.aabb), sphereBounds(other.sphereBounds) { }

    Bounds& operator=(const Bounds& other) {
        if (this != &other) { // protect against invalid self-assignment
            aabb = other.aabb;
            sphereBounds = other.sphereBounds;
        }

        return *this;
    }

    Bounds transform(const glm::mat4& mat);

    const Aabb& getAabb() const {
        return aabb;
    }

    const glm::vec4& getSphereBounds() const {
        return sphereBounds;
    }

    static Bounds fromMinMax(const glm::vec3& min, const glm::vec3& max);
};