#include <kengine/vulkan/Camera.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Camera::Camera(float fov, float aspectRatio, float zNear, float zFar)
    : fov(fov), aspectRatio(aspectRatio), zNear(zNear), zFar(zFar) {
    projection = glm::perspective(fov, aspectRatio, zNear, zFar);
    projection[1][1] *= -1; // invert y-axis for vulkan
}

void Camera::getViewMatrix(glm::mat4& dest) {
    dest = glm::toMat4(rotation);
    dest = glm::translate(dest, -position);
}

void Camera::savePreviousTransform() {
    prevPosition = position;
    prevRotation = rotation;
}

void Camera::getIntegratedViewMatrix(float alpha, glm::mat4& dest) {
    auto t = glm::mix(prevPosition, position, alpha);
    auto r = glm::normalize(glm::mix(prevRotation, rotation, alpha));

    dest = glm::toMat4(r);
    dest = glm::translate(dest, -t);
}

size_t Camera::size() {
    return 16 * 2 * sizeof(float);
}
