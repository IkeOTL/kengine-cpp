#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include <kengine/ecs/EcsSystem.hpp>

#include <thirdparty/entt.hpp>

#include <glm/mat4x4.hpp>

#include <memory>

namespace Component {
    class Renderable;
    class Spatials;
}

class World;
class VulkanContext;
class RenderContext;
class RenderFrameContext;
class AsyncModelCache;
class AsyncMaterialCache;
class CameraController;
class SceneGraph;
class SceneTime;
class Transform;

class RenderSystem : public EcsSystem {
private:
    VulkanContext* vulkanCtx;
    RenderContext* renderCtx;
    AsyncModelCache* modelCache;
    AsyncMaterialCache* materialCache;
    SceneGraph* sceneGraph;
    SceneTime* sceneTime;
    CameraController* cameraController;

public:
    RenderSystem() = default;

    bool checkProcessing() override {
        return false;
    }

    void init() override;
    void processSystem(float delta) override;
    void drawEntities(RenderFrameContext& ctx, float delta);
    void integrate(Component::Renderable& renderable, Component::Spatials& spatials,
        Transform& curTranform, uint32_t meshIdx, float delta, glm::mat4& dest);
};