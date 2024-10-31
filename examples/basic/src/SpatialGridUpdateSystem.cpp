
#include "SpatialGridUpdateSystem.hpp"
#include "components/Components.hpp"
#include "components/Model.hpp"

#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <kengine/SpatialPartitioningManager.hpp>
#include <kengine/Logger.hpp>

void SpatialGridUpdateSystem::init() {
    executorService = getService<ke::ExecutorService>();
    sceneGraph = getService<ke::SceneGraph>();
    modelCache = getService<ke::AsyncModelCache>();
    spatialPartitioning = getService<ke::SpatialPartitioningManager>();
}

void SpatialGridUpdateSystem::processSystem(float delta) {
    auto spatialGrid = spatialPartitioning->getSpatialGrid();

    if (!spatialGrid)
        return;

    spatialGrid->processDirtyEntities([&ecs = this->getEcs(), sceneGraph = this->sceneGraph, modelCache = this->modelCache](entt::entity e) {
        auto& spatialComp = ecs.get<Component::Spatials>(e);
        auto spatial = sceneGraph->get(spatialComp.rootSpatialId);
        auto& current = spatial->getWorldTransform();

        auto& modelComp = ecs.get<Component::ModelComponent>(e);
        // todo: try to async here, this means we have to keep the entity in the dirtyset in case of model not yet loaded
        auto& model = modelCache->get(modelComp.config);

        return ke::SpatialGrid::SpatialGridUpdate{
            e,
            current.getTransMatrix(),
            model.getBounds().getAabb()};
    });
}
