
#include <kengine/game/RenderablePreviousTransformSystem.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/game/components/Material.hpp>
#include <kengine/game/components/Components.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <future>

void RenderablePreviousTransformSystem::init() {
    executorService = getService<ExecutorService>();
    sceneGraph = getService<SceneGraph>();
    cameraController = getService<CameraController>();
}

void RenderablePreviousTransformSystem::processSystem(float delta) {
    auto view = getEcs().view<Component::Renderable, Component::Spatials>();

    cameraController->getCamera()->savePreviousTransform();

    std::vector<std::future<void>> tasks;

    // parallelize
    for (auto& e : view) {
        auto& renderableComponent = view.get<Component::Renderable>(e);

        if (renderableComponent.type != Component::Renderable::DYNAMIC_MODEL)
            continue;

        auto& spatialsComponent = view.get<Component::Spatials>(e);

        auto cnt = spatialsComponent.meshSpatialsIds.size();
        for (auto i = 0; i < cnt; i++)
        {
            auto spatial = sceneGraph->get(spatialsComponent.meshSpatialsIds[i]);
            auto& current = spatial->getWorldTransform();

            spatialsComponent.previousTransforms[i].set(current);
        }
    }
}

