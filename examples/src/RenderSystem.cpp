
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
#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <kengine/util/Random.hpp>

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

        for (size_t i = 0; i < 1; i++) {
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

    // test terrain
    {
        auto* ecs = getWorld().getService<entt::registry>();
        auto tilesWidth = 64;
        auto tilesLength = 64;
        // terrain

        auto tileTerrain = std::make_unique<DualGridTileTerrain>(tilesWidth, tilesLength, 16, 16);

        for (int z = 0; z < tileTerrain->getTerrainHeightsLength(); z++) {
            for (int x = 0; x < tileTerrain->getTerrainHeightsWidth(); x++) {
                tileTerrain->setHeight(x, z, random::randFloat(-.1f, .15f));
            }
        }
        tileTerrain->regenerate(*vulkanCtx, *modelCache);

        auto matConfig = std::make_shared<PbrMaterialConfig>();
        TextureConfig textureConfig("res/img/poke-tileset.png");
        matConfig->addAlbedoTexture(&textureConfig);
        matConfig->setMetallicFactor(0.0f);
        matConfig->setRoughnessFactor(0.5f);
        tileTerrain->setMaterialConfig(matConfig);

        // create entities 
        for (int z = 0; z < tileTerrain->getChunkCountZ(); z++) {
            for (int x = 0; x < tileTerrain->getChunkCountX(); x++) {
                auto& chunk = tileTerrain->getChunk(x, z);
                /*glm::vec4 sphereBounds(
                    -tileTerrain->getTerrainHeightsWidth() * .5f + x * tileTerrain->getChunkWidth() + tileTerrain->getChunkWidth() * .5f,
                    0,
                    -tileTerrain->getTerrainHeightsLength() * .5f + z * tileTerrain->getChunkLength() + tileTerrain->getChunkLength() * .5f,
                    (float)std::sqrt(2 * std::pow(std::max(tileTerrain->getChunkWidth(), tileTerrain->getChunkLength()), 2))
                );*/

                auto entity = ecs->create();
                auto& renderable = ecs->emplace<Component::Renderable>(entity);
                renderable.setStatic();

                auto spatial = sceneGraph->create("terrain: x: " + std::to_string(x) + " z:" + std::to_string(z));
                auto& spatialsComp = ecs->emplace<Component::Spatials>(entity);
                spatialsComp.rootSpatialId = spatial->getSceneId();
                spatialsComp.meshSpatialsIds.push_back(spatialsComp.rootSpatialId);

                auto offset = chunk.getWorldOffset();
                spatial->setLocalPosition(glm::vec3(offset.x, 0, offset.y));

                auto& material = ecs->emplace<Component::Material>(entity, matConfig);
                auto& model = ecs->emplace<Component::ModelComponent>(entity);
                model.config = chunk.getModelConfig();
            }
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
                auto& bounds = m->getBounds();
                renderCtx->draw(*m, *material, node->getWorldTransform().getTransMatrix(), bounds.getSphereBounds());
            }
        }
    }
}
