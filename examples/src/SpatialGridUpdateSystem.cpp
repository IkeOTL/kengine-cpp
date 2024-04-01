
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

    spatialGrid->processDirtyEntities([&ecs = this->getEcs(), sceneGraph = this->sceneGraph, modelCache = this ->modelCache](entt::entity e) {
        auto& spatialComp = ecs.get<Component::Spatials>(e);
        auto spatial = sceneGraph->get(spatialComp.rootSpatialId);
        auto& current = spatial->getWorldTransform();

        auto& modelComp = ecs.get<Component::ModelComponent>(e);
        // todo: try to async here, this means we have to keep the entity in the dirtyset in case of model not yet loaded
        auto& model = modelCache->get(modelComp.config);

        return SpatialGrid::SpatialGridUpdate{
            e,
            current.getTransMatrix(),
            model.getBounds().getAabb()
        };
        });
}

