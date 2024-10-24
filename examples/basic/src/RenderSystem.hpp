#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include <kengine/ecs/EcsSystem.hpp>

#include <thirdparty/entt.hpp>

#include <glm/mat4x4.hpp>

#include <vector>
#include <memory>

namespace Component {
    class Renderable;
    class Spatials;
}

namespace ke {
    class World;
    class DebugContext;
    class VulkanContext;
    class RenderContext;
    class RenderFrameContext;
    class AsyncModelCache;
    class AsyncMaterialCache;
    class CameraController;
    class SceneGraph;
    class SceneTime;
    class Transform;
    class SpatialPartitioningManager;
    class SkeletonManager;
}

class RenderSystem : public ke::EcsSystem {
private:
    ke::DebugContext* debugCtx;
    ke::VulkanContext* vulkanCtx;
    ke::RenderContext* renderCtx;
    ke::AsyncModelCache* modelCache;
    ke::AsyncMaterialCache* materialCache;
    ke::SceneGraph* sceneGraph;
    ke::SceneTime* sceneTime;
    ke::CameraController* cameraController;
    ke::SpatialPartitioningManager* spatialPartitioning;
    ke::SkeletonManager* skeletonManager;

    inline static const uint32_t maxVisibleEntities = 1024;
    std::vector<entt::entity> visibleEntities = std::vector<entt::entity>(maxVisibleEntities);

    void integrate(Component::Renderable& renderable, Component::Spatials& spatials,
        ke::Transform& curTranform, uint32_t meshIdx, float delta, glm::mat4& dest);

public:
    RenderSystem() = default;

    bool checkProcessing() override {
        return false;
    }

    void init() override;
    void processSystem(float delta) override;
    void drawEntities(ke::RenderFrameContext& ctx, float delta);
};