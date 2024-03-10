#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include <kengine/ecs/EcsSystem.hpp>

class World;
class RenderContext;

class RenderSystem : public EcsSystem {
private:
    VulkanContext* vulkanCtx;
    RenderContext* renderCtx;

public:
    RenderSystem() = default;

    void init() override;
    void processSystem(float delta) override;
    void drawEntities(RenderFrameContext& ctx);
};