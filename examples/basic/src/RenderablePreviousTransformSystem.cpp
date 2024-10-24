
#include "RenderablePreviousTransformSystem.hpp"
#include "components/Material.hpp"
#include "components/Components.hpp"

#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <future>

void RenderablePreviousTransformSystem::init() {
    executorService = getService<ke::ExecutorService>();
    sceneGraph = getService<ke::SceneGraph>();
    cameraController = getService<ke::CameraController>();
}

void RenderablePreviousTransformSystem::processSystem(float delta) {
    ZoneScoped;

    cameraController->getCamera()->savePreviousTransform();

    // store previoius transform for renderables
    {
        auto view = getEcs().view<Component::Renderable, Component::Spatials>();
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

    // store previoius transform for skeletons
    {
        auto view = getEcs().view<Component::SkeletonComp>();
        // parallelize
        for (auto& e : view) {
            auto& skeletonComponent = view.get<Component::SkeletonComp>(e);
            auto s = sceneGraph->get(skeletonComponent.skeletonId);
            auto skeleton = std::static_pointer_cast<ke::Skeleton>(sceneGraph->get(skeletonComponent.skeletonId));
            skeleton->savePreviousTransforms();
        }
    }
}

