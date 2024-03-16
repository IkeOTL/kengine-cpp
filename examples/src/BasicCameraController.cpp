#include <kengine/game/BasicCameraController.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


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
            glm::quat rot = getCamera()->getRotation();
            rot = glm::rotate(rot, glm::radians(0.1f * deltaX), glm::vec3(0, 1, 0));
            rot = glm::rotate(rot, glm::radians(0.1f * deltaY), glm::vec3(1, 0, 0));
            getCamera()->setRotation(rot);

            return false;
        });
}

void FreeCameraController::update(float delta) {
    // update frustum stuffs
}
void FreeCameraController::setPosition(float x, float y, float z) {
    camera->setPosition(glm::vec3(x, y, z));
}