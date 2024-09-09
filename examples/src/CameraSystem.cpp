
#include <kengine/game/CameraSystem.hpp>
#include <kengine/game/BasicCameraController.hpp>

void CameraSystem::init() {
    cameraController = getService<CameraController>();
}

void CameraSystem::processSystem(float delta) {
    cameraController->update(delta);
}