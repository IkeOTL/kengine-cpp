#include <kengine/Transform.hpp>

namespace ke {
    void Transform::setDirty(bool isDirty) {
        dirty = isDirty;
    }

    void Transform::set(const Transform& in) {
        set(in.position, in.rotation, in.scale);
    }

    void Transform::set(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl) {
        position = pos;
        rotation = rot;
        scale = scl;
        setDirty(true);
    }

    const glm::mat4& Transform::getTransMatrix() {
        if (dirty)
            updateTransform();

        return transMatrix;
    }

    void Transform::setPosition(const glm::vec3& p) {
        position = p;
        dirty = true;
    }

    void Transform::setPosition(float x, float y, float z) {
        position = glm::vec3(x, y, z);
        dirty = true;
    }

    void Transform::setScale(const glm::vec3& s) {
        scale = s;
        dirty = true;
    }

    void Transform::setScale(float x, float y, float z) {
        scale = glm::vec3(x, y, z);
        dirty = true;
    }

    void Transform::setRotation(const glm::quat& q) {
        rotation = glm::normalize(q);
        dirty = true;
    }

    void Transform::setRotation(float x, float y, float z, float w) {
        rotation = glm::normalize(glm::quat(w, x, y, z));
        dirty = true;
    }

    void Transform::slerpRotation(const glm::quat& target, float alpha) {
        rotation = glm::slerp(rotation, target, alpha);
        dirty = true;
    }

    const glm::vec3& Transform::getPosition() {
        if (dirty)
            updateTransform();

        return position;
    }

    const glm::vec3& Transform::getScale() {
        if (dirty)
            updateTransform();

        return scale;
    }

    const glm::quat& Transform::getRotation() {
        if (dirty)
            updateTransform();

        return rotation;
    }

    void Transform::addPosition(const glm::vec3& v) {
        position += v;
        dirty = true;
    }

    void Transform::addPosition(float x, float y, float z) {
        position += glm::vec3(x, y, z);
        dirty = true;
    }

    void Transform::rotate(float angle, const glm::vec3& axis) {
        rotation = glm::rotate(rotation, glm::radians(angle), axis);
        dirty = true;
    }

    void Transform::updateTransform() {
        rotation = glm::normalize(rotation);
        transMatrix = glm::translate(glm::mat4(1.0f), position) *
            glm::mat4_cast(rotation) *
            glm::scale(glm::mat4(1.0f), scale);
        dirty = false;
    }

    void Transform::updateTransform(Transform& parent) {
        position = glm::vec3(parent.getTransMatrix() * glm::vec4(position, 1.0f));
        rotation = glm::normalize(parent.rotation * rotation);
        scale *= parent.scale;
        updateTransform();
    }

    void Transform::interpolate(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl, float factor) {
        position = glm::mix(position, pos, factor);
        rotation = glm::normalize(glm::lerp(rotation, rot, factor));
        scale = glm::mix(scl, pos, factor);
        dirty = true;
    }
} // namespace ke