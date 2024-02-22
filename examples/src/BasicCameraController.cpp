#include <kengine/game/BasicCameraController.hpp>


void BasicCameraController::update(float delta) {
    // update frustum stuffs
}
void BasicCameraController::setPosition(float x, float y, float z) {
    camera->setPosition(glm::vec3(x, y, z));
}