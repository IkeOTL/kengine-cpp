#pragma once
#include <kengine/ecs/EcsSystem.hpp>

namespace ke {
    class World;
    class SceneGraph;
    class Transform;
    class ExecutorService;
    class AsyncModelCache;
    class SpatialPartitioningManager;
}

class SpatialGridUpdateSystem : public ke::EcsSystem {
private:
    ke::ExecutorService* executorService = nullptr;
    ke::SceneGraph* sceneGraph = nullptr;
    ke::AsyncModelCache* modelCache = nullptr;
    ke::SpatialPartitioningManager* spatialPartitioning = nullptr;

public:
    SpatialGridUpdateSystem() = default;

    void init() override;
    void processSystem(float delta) override;
};