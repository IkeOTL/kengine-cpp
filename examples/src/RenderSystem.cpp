
#include <kengine/game/RenderSystem.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/ecs/World.hpp>


void RenderSystem::init() {
    vulkanCtx = getWorld().getService<VulkanContext>();
    renderCtx = getWorld().getService<RenderContext>();
}

void RenderSystem::processSystem(float delta) {
    auto ctx = vulkanCtx->createNextFrameContext();
    renderCtx->begin(*ctx, 0, 0);
    {
        drawEntities(*ctx);
    }
    renderCtx->end();

}


void RenderSystem::drawEntities(RenderFrameContext& ctx) {
    auto view = getEcs().view<Component::Model>();

    for (auto& e : view) {
        auto& model = view.get<Component::Model>(e);
        auto* lol = this;
    }
}
