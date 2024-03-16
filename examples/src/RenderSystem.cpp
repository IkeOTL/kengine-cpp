
#include <kengine/game/RenderSystem.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/game/Game.hpp>
#include <kengine/game/components/Material.hpp>
#include <kengine/game/components/Model.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>

void RenderSystem::init() {
    vulkanCtx = getService<VulkanContext>();
    renderCtx = getService<RenderContext>();
    modelCache = getService<AsyncModelCache>();
    materialCache = getService<AsyncMaterialCache>();
    sceneTime = getService<SceneTime>();

    // test obj
    {
        auto* ecs = getWorld().getService<entt::registry>();
        auto modelConfig = std::make_shared<ModelConfig>("res/gltf/char01.glb",
            VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
            | VertexAttribute::TANGENTS
        );

        auto materialConfig = std::make_shared<PbrMaterialConfig>();

        auto entity = ecs->create();
        ecs->emplace<Component::Model>(entity, modelConfig);
        ecs->emplace<Component::Material>(entity, materialConfig);
    }
}

void RenderSystem::processSystem(float delta) {
    auto ctx = vulkanCtx->createNextFrameContext();
    renderCtx->begin(*ctx, sceneTime->getSceneTime(), delta);
    {
        drawEntities(*ctx);
    }
    renderCtx->end();
}

void RenderSystem::drawEntities(RenderFrameContext& ctx) {
    auto view = getEcs().view<Component::Model, Component::Material>();

    for (auto& e : view) {
        auto& modelComponent = view.get<Component::Model>(e);
        auto modelTask = modelCache->getAsync(modelComponent.config);

        auto& materialComponent = view.get<Component::Material>(e);
        auto materialTask = materialCache->getAsync(materialComponent.config);

        // maybe use a default material on a mesh if material isnt ready
        if (!modelTask.isDone())
            continue;

        if (!materialTask.isDone())
            continue;

        auto model = modelTask.get();
        auto material = materialTask.get();
        renderCtx->draw(model->getAMesh(), *material, glm::mat4(1), glm::vec4(0, 0, 0, 1));
    }
}
