#include <kengine/game/BasicCameraController.hpp>
#include <kengine/util/VecUtils.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


void BasicCameraController::update(float delta) {
    // update frustum stuffs
}
void BasicCameraController::setPosition(float x, float y, float z) {
    camera->setPosition(glm::vec3(x, y, z));
}

void FreeCameraController::init() {
    inputManager.registerMouseEventListener(this);
    addDragHandler([this](GLFWwindow* window, int x, int y, int deltaX, int deltaY)
        {
            /* auto orientation = getCamera()->getRotation();

             auto rightAxis = orientation * glm::vec3(1.0f, 0.0f, 0.0f);
             auto pitchRotation = glm::angleAxis(glm::radians(0.1f * deltaY), rightAxis);
             orientation = pitchRotation * orientation;

             auto yawRotation = glm::angleAxis(glm::radians(0.1f * deltaX), glm::vec3(0.0f, 1.0f, 0.0f));
             orientation = yawRotation * orientation;

             getCamera()->setRotation(orientation);*/

            glm::quat orientation = getCamera()->getRotation();
            //auto right = glm::axis(orientation);

            // Calculate rotation quaternions
            auto yaw = glm::angleAxis(glm::radians(0.1f * static_cast<float>(deltaX)), glm::vec3(0.0f, 1.0f, 0.0f));
            auto pitch = glm::angleAxis(glm::radians(0.1f * static_cast<float>(deltaY)), yaw * glm::vec3(1.0f, 0.0f, 0.0f));

            // Combine the new rotations with the current orientation
            orientation = glm::normalize(pitch * orientation * yaw);
            getCamera()->setRotation(orientation);

            return false;
        });
}

void FreeCameraController::update(float delta) {
    auto hasMovement = inputManager.isKeyDown(GLFW_KEY_A) || inputManager.isKeyDown(GLFW_KEY_D)
        || inputManager.isKeyDown(GLFW_KEY_LEFT_SHIFT) || inputManager.isKeyDown(GLFW_KEY_SPACE)
        || inputManager.isKeyDown(GLFW_KEY_W) || inputManager.isKeyDown(GLFW_KEY_S);

    if (!hasMovement)
        return;

    auto* camera = getCamera();

    if (!camera)
        return;

    auto camMov = glm::vec3(
        (inputManager.isKeyDown(GLFW_KEY_A) ? -1 : 0) + (inputManager.isKeyDown(GLFW_KEY_D) ? 1 : 0),
        0,
        (inputManager.isKeyDown(GLFW_KEY_W) ? -1 : 0) + (inputManager.isKeyDown(GLFW_KEY_S) ? 1 : 0)
    );

    auto invRot = glm::inverse(camera->getRotation());
    camMov = glm::rotate(invRot, camMov);

    camMov.y += (inputManager.isKeyDown(GLFW_KEY_LEFT_SHIFT) ? -1 : 0) + (inputManager.isKeyDown(GLFW_KEY_SPACE) ? 1 : 0);

    camMov = glm::normalize(camMov);
    if (vecutils::isFinite(camMov)) {
        camMov *= 3.0f * delta;
        camera->addPosition(camMov);
    }
}
void FreeCameraController::setPosition(float x, float y, float z) {
    camera->setPosition(glm::vec3(x, y, z));
}