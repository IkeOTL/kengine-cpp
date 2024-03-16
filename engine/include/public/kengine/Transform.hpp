#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
protected:
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation = glm::identity<glm::quat>();

    glm::mat4 transMatrix = glm::mat4(1.0f);

    bool dirty = true;

public:
    Transform()
        : position(0.0f), scale(1.0f), rotation(glm::identity<glm::quat>()) {}

    Transform(const glm::vec3 p)
        : position(p), scale(1.0f), rotation(glm::identity<glm::quat>()) {}

    Transform(const glm::vec3 p, const glm::vec3 s, const glm::quat r)
        : position(p), scale(s), rotation(r) {}

    Transform(const Transform& t) {
        set(t);
    }

    void setDirty(bool isDirty);
    void set(const Transform& in);
    const glm::mat4& getTransMatrix();
    void setPosition(const glm::vec3& p);
    void setPosition(float x, float y, float z);
    void setScale(const glm::vec3& s);
    void setScale(float x, float y, float z);
    void setRotation(const glm::quat& q);
    void setRotation(float x, float y, float z, float w);
    void slerpRotation(const glm::quat& target, float alpha);
    glm::vec3 getPosition();
    glm::vec3 getScale();
    glm::quat getRotation();
    void addPosition(const glm::vec3& v);
    void addPosition(float x, float y, float z);
    void rotate(float angle, const glm::vec3& axis);
    void updateTransform();
    void updateTransform(Transform& parent);
};