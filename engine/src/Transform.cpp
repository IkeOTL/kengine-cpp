#include <kengine/Transform.hpp>

void Transform::setDirty(bool isDirty) {
    dirty = isDirty;
}

void Transform::set(const Transform& in) {
    position = in.position;
    rotation = in.rotation;
    scale = in.scale;
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

glm::vec3 Transform::getPosition() {
    if (dirty)
        updateTransform();

    return position;
}

glm::vec3 Transform::getScale() {
    if (dirty)
        updateTransform();

    return scale;
}

glm::quat Transform::getRotation() {
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
        glm::toMat4(rotation) *
        glm::scale(glm::mat4(1.0f), scale);
    dirty = false;
}

void Transform::updateTransform(Transform& parent) {
    position = glm::vec3(parent.getTransMatrix() * glm::vec4(position, 1.0f));
    rotation = glm::normalize(parent.rotation * rotation);
    scale *= parent.scale;
    updateTransform();
}