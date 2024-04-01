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
    ExecutorService* executorService;
    SceneGraph* sceneGraph;
    AsyncModelCache* modelCache;
    SpatialPartitioningManager* spatialPartitioning;

public:
    SpatialGridUpdateSystem() = default;

    bool checkProcessing() override {
        return false;
    }

    void init() override;
    void processSystem(float delta) override;
};