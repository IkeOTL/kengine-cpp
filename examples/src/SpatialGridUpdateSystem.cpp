
#include <kengine/game/SpatialGridUpdateSystem.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/ecs/World.hpp>
#include <kengine/game/components/Components.hpp>
#include <kengine/game/components/Model.hpp>
#include <thirdparty/entt.hpp>
#include <kengine/SpatialPartitioningManager.hpp>

void SpatialGridUpdateSystem::init() {
    executorService = getService<ExecutorService>();
    sceneGraph = getService<SceneGraph>();
    modelCache = getService<AsyncModelCache>();
    spatialPartitioning = getService<SpatialPartitioningManager>();
}

void SpatialGridUpdateSystem::processSystem(float delta) {
    auto spatialGrid = spatialPartitioning->getSpatialGrid();

    if (!spatialGrid)
        return;

    auto& ecs = getEcs();
    spatialGrid->processDirtyEntities([&ecs, sceneGraph = this->sceneGraph, modelCache = this ->modelCache](entt::entity e) {
        auto& spatialComp = ecs.get<Component::Spatials>(e);
        auto spatial = sceneGraph->get(spatialComp.rootSpatialId);
        auto& current = spatial->getWorldTransform();

        auto& modelComp = ecs.get<Component::ModelComponent>(e);
        auto& model = modelCache->get(modelComp.config);

        return SpatialGrid::SpatialGridUpdate{
            e,
            current,
            model.getBounds().getAabb()
        };
        });
}

