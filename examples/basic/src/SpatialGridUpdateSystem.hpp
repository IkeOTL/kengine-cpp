#pragma once
#include <kengine/ecs/EcsSystem.hpp>

class World;
class SceneGraph;
class Transform;
class ExecutorService;
class AsyncModelCache;
class SpatialPartitioningManager;

class SpatialGridUpdateSystem : public EcsSystem {
private:
    ExecutorService* executorService = nullptr;
    SceneGraph* sceneGraph = nullptr;
    AsyncModelCache* modelCache = nullptr;
    SpatialPartitioningManager* spatialPartitioning = nullptr;

public:
    SpatialGridUpdateSystem() = default;

    void init() override;
    void processSystem(float delta) override;
};