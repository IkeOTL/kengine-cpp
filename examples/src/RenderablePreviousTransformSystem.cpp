
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

    std::vector<std::future<void>> tasks;
    tasks.reserve(view.size_hint());

    for (auto& e : view) {
        auto& renderableComponent = view.get<Component::Renderable>(e);

        if (renderableComponent.type != Component::Renderable::DYNAMIC_MODEL)
            continue;

        auto& spatialsComponent = view.get<Component::Spatials>(e);

        // root spatial
        {
            auto spatial = sceneGraph->get(spatialsComponent.rootSpatialId);
            auto& current = spatial.getWorldTransform();

            // we should only care about alpha lerping dynamic objects
            if (renderableComp.type == RenderableComponent.RenderableType.DYNAMIC_MODEL) {
                var previous = transformStore.get(spatialComp.rootSpatialId);
                previous.set(current);
            }

            //                        var modelComp = mModel.get(eId);
            //                        var model = modelCache.get(modelComp.modelConfig).join();
        }
    }


}

