#include <kengine/vulkan/Camera.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float fov, float aspectRatio, float zNear, float zFar)
    : fov(fov), aspectRatio(aspectRatio), zNear(zNear), zFar(zFar) {
    projection = glm::perspective(fov, aspectRatio, zNear, zFar);
    projection[1][1] *= -1; // invert y-axis for vulkan
}

void Camera::getViewMatrix(glm::mat4& dest) {
    dest = glm::mat4_cast(rotation);
    dest = glm::translate(dest, -position);
}

void Camera::savePreviousTransform() {
    prevPosition = position;
    prevRotation = rotation;
}

void Camera::getIntegratedViewMatrix(float alpha, glm::mat4& dest) {
    auto t = glm::mix(prevPosition, position, alpha);
    
    // interpolating the rotation is causing jittering??
    // likely because the window input is polled once per frame, while camera movement is updated every fixed timestep??
    //auto r = glm::normalize(glm::mix(prevRotation, rotation, alpha));
    //dest = glm::mat4_cast(r);

    dest = glm::mat4_cast(rotation);
    dest = glm::translate(dest, -t);
}

size_t Camera::size() {
    return 16 * 2 * sizeof(float);
}
