
#include <kengine/game/RenderSystem.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/game/Game.hpp>
#include <kengine/game/components/Material.hpp>
#include <kengine/game/components/Model.hpp>
#include <kengine/game/components/Components.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>

void RenderSystem::init() {
    vulkanCtx = getService<VulkanContext>();
    renderCtx = getService<RenderContext>();
    modelCache = getService<AsyncModelCache>();
    materialCache = getService<AsyncMaterialCache>();
    sceneGraph = getService<SceneGraph>();
    sceneTime = getService<SceneTime>();

    // test obj
    {
        auto* ecs = getWorld().getService<entt::registry>();
        auto modelConfig = std::make_shared<ModelConfig>("res/gltf/char01.glb",
            VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
            | VertexAttribute::TANGENTS
        );

        auto materialConfig = std::make_shared<PbrMaterialConfig>();

        for (size_t i = 0; i < 10; i++) {
            auto entity = ecs->create();
            auto& renderable = ecs->emplace<Component::Renderable>(entity);
            ecs->emplace<Component::ModelComponent>(entity, modelConfig);
            ecs->emplace<Component::Material>(entity, materialConfig);

            auto& model = modelCache->get(modelConfig);
            auto& spatials = ecs->emplace<Component::Spatials>(entity);
            spatials.generate(*sceneGraph, model, "player" + std::to_string(i));

            auto rootSpatial = sceneGraph->get(spatials.rootSpatialId);
            rootSpatial->setLocalPosition(glm::vec3(3.0f * i, .1337f, 0));
        }
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
    auto view = getEcs().view<Component::Renderable, Component::Spatials, Component::ModelComponent, Component::Material>();

    for (auto& e : view) {
        auto& modelComponent = view.get<Component::ModelComponent>(e);
        auto modelTask = modelCache->getAsync(modelComponent.config);

        auto& materialComponent = view.get<Component::Material>(e);
        auto materialTask = materialCache->getAsync(materialComponent.config);

        // maybe use a default material on a mesh if material isnt ready
        if (!modelTask.isDone())
            continue;

        if (!materialTask.isDone())
            continue;

        auto& renderableComponent = view.get<Component::Renderable>(e);
        auto& spatialsComponent = view.get<Component::Spatials>(e);

        auto model = modelTask.get();
        auto material = materialTask.get();

        auto curIdx = 0;
        for (const auto& mg : model->getMeshGroups()) {
            for (const auto& m : mg->getMeshes()) {
                auto node = sceneGraph->get(spatialsComponent.meshSpatialsIds[curIdx++]);

                // need to calc in Model still
                auto tmpBounds = glm::vec4(0, 0, 0, 1);
                renderCtx->draw(*m, *material, node->getWorldTransform().getTransMatrix(), tmpBounds);
            }
        }
    }
}
