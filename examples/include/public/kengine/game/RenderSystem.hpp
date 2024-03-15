#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include <kengine/ecs/EcsSystem.hpp>

#include <thirdparty/entt.hpp>

#include <memory>

class World;
class VulkanContext;
class RenderContext;
class RenderFrameContext;
class AsyncModelCache;
class AsyncMaterialCache;
class SceneTime;

class RenderSystem : public EcsSystem {
private:
    VulkanContext* vulkanCtx;
    RenderContext* renderCtx;
    AsyncModelCache* modelCache;
    AsyncMaterialCache* materialCache;
    SceneTime* sceneTime;

public:
    RenderSystem() = default;

    bool checkProcessing() override {
        return false;
    }

    void init() override;
    void processSystem(float delta) override;
    void drawEntities(RenderFrameContext& ctx);
};