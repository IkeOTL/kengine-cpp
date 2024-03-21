
#include <kengine/game/RenderSystem.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
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
    cameraController = getService<CameraController>();

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
            spatials.previousTransforms.resize(spatials.meshSpatialsIds.size());

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

                auto entity = ecs->create();
                auto& renderable = ecs->emplace<Component::Renderable>(entity);
                renderable.setStatic();

                auto spatial = sceneGraph->create("terrain: x: " + std::to_string(x) + " z:" + std::to_string(z));
                auto& spatialsComp = ecs->emplace<Component::Spatials>(entity);
                spatialsComp.rootSpatialId = spatial->getSceneId();
                spatialsComp.meshSpatialsIds.push_back(spatialsComp.rootSpatialId);
                spatialsComp.previousTransforms.resize(spatialsComp.meshSpatialsIds.size());

                auto offset = chunk.getWorldOffset();
                spatial->setLocalPosition(glm::vec3(offset.x, 0, offset.y));

                auto& material = ecs->emplace<Component::Material>(entity, matConfig);
                auto& model = ecs->emplace<Component::ModelComponent>(entity);
                model.config = chunk.getModelConfig();

                // need to profile if static batches are even worth it.
                // since the drawcmd is always sent for them. the ebefit is that
                // the mat and other details dont have to be uploaded again
                //{
                //    auto modelTask = modelCache->getAsync(model.config);
                //    auto materialTask = materialCache->getAsync(material.config);

                //    auto model = modelTask.get();
                //    auto material = materialTask.get();
                //    renderCtx->addStaticInstance(
                //        model->getMeshGroups()[0]->getMesh(0),
                //        *material,
                //        spatial->getWorldTransform().getTransMatrix(),
                //        glm::vec4(0, 0, 0, 2),
                //        false
                //    );
                //}
            }
        }
    }
}

void RenderSystem::processSystem(float delta) {
    auto ctx = vulkanCtx->createNextFrameContext();
    renderCtx->begin(*ctx, sceneTime->getSceneTime(), delta);
    {
        drawEntities(*ctx, delta);
    }
    renderCtx->end();
}


void RenderSystem::integrate(bool shouldIntegrate, Transform& prevTransform, Transform& curTranform, float delta, glm::mat4& dest) {
    if (!shouldIntegrate) {
        dest = curTranform.getTransMatrix();
        return;
    }

    auto position = glm::mix(prevTransform.getPosition(), curTranform.getPosition(), delta);
    auto rotation = glm::lerp(prevTransform.getRotation(), curTranform.getRotation(), delta);
    auto scale = glm::mix(prevTransform.getScale(), curTranform.getScale(), delta);

    rotation = glm::normalize(rotation);
    dest = glm::translate(glm::mat4(1.0f), position) *
        glm::toMat4(rotation) *
        glm::scale(glm::mat4(1.0f), scale);
}

void RenderSystem::drawEntities(RenderFrameContext& ctx, float delta) {
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

        //if (renderableComponent.type == Component::Renderable::STATIC_MODEL)
        //    continue;

        auto& spatialsComponent = view.get<Component::Spatials>(e);

        auto model = modelTask.get();
        auto material = materialTask.get();

        glm::mat4 blendMat{};
        auto curIdx = 0;
        for (const auto& mg : model->getMeshGroups()) {
            for (const auto& m : mg->getMeshes()) {
                auto node = sceneGraph->get(spatialsComponent.meshSpatialsIds[curIdx]);
                auto& prevTransform = spatialsComponent.previousTransforms[curIdx];
                auto& curTranform = node->getWorldTransform();

                integrate(renderableComponent.integrateRendering, prevTransform, curTranform, delta, blendMat);

                // need to calc in Model still
                auto& bounds = m->getBounds();
                renderCtx->draw(*m, *material, blendMat, bounds.getSphereBounds());

                curIdx++;
            }
        }
    }
}
