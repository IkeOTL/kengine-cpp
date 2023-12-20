#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {

private:
    glm::vec3 prevPosition{};
    glm::quat prevRotation{};

    glm::vec3 position{};
    glm::quat rotation{};

    glm::mat4 projection{};

    const float fov, zNear, zFar, aspectRatio;

public:
    const static float NEAR, FAR;

    Camera(float fov, float aspectRatio, float zNear, float zFar);

    void getViewMatrix(glm::mat4& dest);
    void savePreviousTransform();
    void getIntegratedViewMatrix(float alpha, glm::mat4& dest);

    float getFov() {
        return fov;
    }

    float getNearClip() {
        return zNear;
    }

    float getFarClip() {
        return zFar;
    }

    float getAspectRatio() {
        return aspectRatio;
    }

    glm::mat4& getProjectionMatrix() {
        return projection;
    }

    glm::vec3& getPosition() {
        return position;
    }

    glm::quat& getRotation() {
        return rotation;
    }

    static size_t size();
};