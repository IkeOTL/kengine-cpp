#include "BasicCameraController.hpp"

#include <kengine/util/VecUtils.hpp>
#include <kengine/util/MatUtils.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <kengine/Logger.hpp>

void BasicCameraController::update(float delta) {
    // update frustum stuffs
}
void BasicCameraController::setPosition(float x, float y, float z) {
    camera->setPosition(glm::vec3(x, y, z));
}

FreeCameraController::FreeCameraController(InputManager& inputManager)
    : InputEventAdapter(inputManager) {
    init();
}

void FreeCameraController::init() {
    inputManager.registerMouseEventListener(this);
    addDragHandler([this](GLFWwindow* window, int x, int y, int deltaX, int deltaY)
        {
            // this starts to roll the camera over time, need to find a fix
            glm::quat orientation = getCamera()->getRotation();

            // Calculate rotation quaternions
            auto yaw = glm::angleAxis(glm::radians(0.1f * deltaX), glm::vec3(0.0f, 1.0f, 0.0f));
            auto pitch = glm::angleAxis(glm::radians(0.1f * deltaY), glm::vec3(1.0f, 0.0f, 0.0f));

            // Combine the new rotations with the current orientation
            orientation = glm::normalize(pitch * orientation * yaw);

            getCamera()->setRotation(orientation);

            needsFrustumRecalc = true;

            return false;
        });
}

void FreeCameraController::update(float delta) {
    if (!camera)
        return;

    applyMovement(delta);

    // frustum update
    if (!needsFrustumRecalc)
        return;

    // frustum recalc
    glm::mat4 projViewMat;
    camera->getViewMatrix(projViewMat);
    projViewMat = camera->getProjectionMatrix() * projViewMat;

    frustumTester.set(projViewMat, false);

    for (int i = 0; i < 8; i++)
        matutils::frustumCorner(projViewMat, static_cast<matutils::FrustumCorner>(i), frustumCorners[i]);

    auto invProjview = glm::inverse(projViewMat);
    matutils::frustumAabb(invProjview, frustumMin, frustumMax);

    needsFrustumRecalc = false;
}

void FreeCameraController::applyMovement(float delta) {
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
    camMov = invRot * camMov;

    camMov.y += (inputManager.isKeyDown(GLFW_KEY_LEFT_SHIFT) ? -1 : 0) + (inputManager.isKeyDown(GLFW_KEY_SPACE) ? 1 : 0);

    camMov = glm::normalize(camMov);
    if (!vecutils::isFinite(camMov))
        return;

    auto boost = inputManager.isMouseButtonDown(GLFW_MOUSE_BUTTON_1) && inputManager.isMouseButtonDown(GLFW_MOUSE_BUTTON_2) ? 3 : 1;

    camMov *= 3.0f * delta * boost;
    camera->addPosition(camMov);
    needsFrustumRecalc = true;
}

void FreeCameraController::setPosition(float x, float y, float z) {
    camera->setPosition(glm::vec3(x, y, z));
    needsFrustumRecalc = true;
}