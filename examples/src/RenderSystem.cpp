
#include <kengine/game/RenderSystem.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>


void RenderSystem::init() {
    vulkanCtx = getWorld().getService<VulkanContext>();
    renderCtx = getWorld().getService<RenderContext>();
    modelCache = getWorld().getService<AsyncModelCache>();

    // test obj
    {
        auto* ecs = getWorld().getService<entt::registry>();
        auto modelConfig = std::make_shared<ModelConfig>("res/gltf/char01.glb",
            VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
            | VertexAttribute::TANGENTS | VertexAttribute::SKELETON
        );

        auto entity = ecs->create();
        ecs->emplace<Component::Model>(entity, modelConfig);
    }
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
        auto& modelComponent = view.get<Component::Model>(e);
        auto modelTask = modelCache->getAsync(modelComponent.config);

        if (!modelTask.isDone())
            continue;

        auto model = modelTask.get();
        auto lol = "";
        //  renderCtx->draw()
    }
}
