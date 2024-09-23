
#include "CameraSystem.hpp"
#include "BasicCameraController.hpp"

void CameraSystem::init() {
    cameraController = getService<CameraController>();
}

void CameraSystem::processSystem(float delta) {
    cameraController->update(delta);
}