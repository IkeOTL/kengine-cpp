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
    glm::quat rotation;

    glm::mat4 transMatrix = glm::mat4(1.0f);

    bool dirty = true;

public:
    Transform()
        : position(0.0f), scale(1.0f), rotation(glm::quat(1.0, 0.0, 0.0, 0.0)) {}

    Transform(const glm::vec3 p)
        : position(p), scale(1.0f), rotation(glm::quat(1.0, 0.0, 0.0, 0.0)) {}

    Transform(const glm::vec3 p, const glm::vec3 s, const glm::quat r)
        : position(p), scale(s), rotation(r) {}

    Transform(const glm::mat4& m) {
        set(m);
    }

    Transform(const Transform& t) {
        set(t);
    }

    void setDirty(bool isDirty) {
        dirty = isDirty;
    }

    void set(const Transform& in) {
        position = in.position;
        rotation = in.rotation;
        scale = in.scale;
        setDirty(true);
    }

    glm::mat4 getTransMatrix() {
        if (dirty)
            updateTransform();

        return transMatrix;
    }

    void setPosition(const glm::vec3& p) {
        position = p;
        dirty = true;
    }

    void setPosition(float x, float y, float z) {
        position = glm::vec3(x, y, z);
        dirty = true;
    }

    void setScale(const glm::vec3& s) {
        scale = s;
        dirty = true;
    }

    void setScale(float x, float y, float z) {
        scale = glm::vec3(x, y, z);
        dirty = true;
    }

    void setRotation(const glm::quat& q) {
        rotation = glm::normalize(q);
        dirty = true;
    }

    void setRotation(float x, float y, float z, float w) {
        rotation = glm::normalize(glm::quat(w, x, y, z));
        dirty = true;
    }

    void slerpRotation(const glm::quat& target, float alpha) {
        rotation = glm::slerp(rotation, target, alpha);
        dirty = true;
    }

    glm::vec3 getPosition() {
        if (dirty)
            updateTransform();

        return position;
    }

    glm::vec3 getScale() {
        if (dirty)
            updateTransform();

        return scale;
    }

    glm::quat getRotation() {
        if (dirty)
            updateTransform();

        return rotation;
    }

    void addPosition(const glm::vec3& v) {
        position += v;
        dirty = true;
    }

    void addPosition(float x, float y, float z) {
        position += glm::vec3(x, y, z);
        dirty = true;
    }

    void rotate(float angle, const glm::vec3& axis) {
        rotation = glm::rotate(rotation, glm::radians(angle), axis);
        dirty = true;
    }

    void updateTransform() {
        rotation = glm::normalize(rotation);
        transMatrix = glm::translate(glm::mat4(1.0f), position) *
            glm::toMat4(rotation) *
            glm::scale(glm::mat4(1.0f), scale);
        dirty = false;
    }

    void updateTransform(Transform& parent) {
        position = glm::vec3(parent.getTransMatrix() * glm::vec4(position, 1.0f));
        rotation = glm::normalize(parent.rotation * rotation);
        scale *= parent.scale;
        updateTransform();
    }
};