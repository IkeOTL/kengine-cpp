
#include <kengine/game/RenderSystem.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/DebugContext.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/game/Game.hpp>
#include <kengine/game/components/Material.hpp>
#include <kengine/game/components/Model.hpp>
#include <kengine/game/components/Components.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/SpatialPartitioningManager.hpp>
#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <kengine/util/Random.hpp>
#include <kengine/game/BasicCameraController.hpp>
#include <kengine/vulkan/SkeletonManager.hpp>
#include <tracy/Tracy.hpp>
#include <kengine/util/MatUtils.hpp>
#include <kengine/EngineConfig.hpp>

void RenderSystem::init() {
    vulkanCtx = getService<VulkanContext>();
    debugCtx = getService<DebugContext>();
    renderCtx = getService<RenderContext>();
    modelCache = getService<AsyncModelCache>();
    materialCache = getService<AsyncMaterialCache>();
    sceneGraph = getService<SceneGraph>();
    sceneTime = getService<SceneTime>();
    cameraController = getService<CameraController>();
    spatialPartitioning = getService<SpatialPartitioningManager>();
    skeletonManager = getService<SkeletonManager>();

    // test obj
    {
        auto* ecs = getWorld().getService<entt::registry>();
        auto modelConfig = std::make_shared<ModelConfig>("res/gltf/smallcube.glb",
            VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
            | VertexAttribute::TANGENTS
        );

        auto materialConfig = PbrMaterialConfig::create();
        materialConfig->setHasShadow(true);

        auto xCount = 20;
        auto zCount = 20;
        auto zOffset = -5;
        for (size_t i = 0; i < xCount; i++) {
            for (size_t j = 0; j < zCount; j++) {
                auto entity = ecs->create();
                auto& renderable = ecs->emplace<Component::Renderable>(entity);
                renderable.setStatic();
                ecs->emplace<Component::ModelComponent>(entity, modelConfig);

                auto& model = modelCache->get(modelConfig);
                auto& spatials = ecs->emplace<Component::Spatials>(entity);
                auto rootSpatial = spatials.generate(*sceneGraph, model, "player" + std::to_string(i), renderable.type);

                //rootSpatial->setChangeCb(spatialPartitioning->getSpatialGrid()->createCb(entity));

                rootSpatial->setLocalPosition(glm::vec3((1.5f * i) - (1.5 * xCount * 0.5f), 3, (1.5f * j) - (1.5 * zCount * 0.5f) + zOffset));

                spatialPartitioning->getSpatialGrid()->setDirty(entity);

                ecs->emplace<Component::Material>(entity, materialConfig);
            }
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

        auto matConfig = PbrMaterialConfig::create();
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
                spatial->setChangeCb(spatialPartitioning->getSpatialGrid()->createCb(entity));
                auto& spatialsComp = ecs->emplace<Component::Spatials>(entity);
                spatialsComp.rootSpatialId = spatial->getSceneId();
                spatialsComp.meshSpatialsIds.push_back(spatialsComp.rootSpatialId);

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
    ZoneScoped;

    auto ctx = vulkanCtx->createNextFrameContext();
    renderCtx->begin(*ctx, sceneTime->getSceneTime(), delta);
    {
        drawEntities(*ctx, delta);
    }
    renderCtx->end();
}

static glm::vec3 intersectPoint(const glm::vec3& start, const glm::vec3& end) {
    // ray never intersects with floor plane
    // lets default the intersection happening at the end of the ray
    if ((start.y > 0 && end.y > 0) || (start.y < 0 && end.y < 0))
        return glm::vec3(end.x, 0, end.z);

    auto factor = start.y / (start.y - end.y);

    return glm::mix(start, end, factor);
}

void RenderSystem::integrate(Component::Renderable& renderable, Component::Spatials& spatials,
    Transform& curTranform, uint32_t meshIdx, float delta, glm::mat4& dest) {
    ZoneScoped;

    if (!renderable.integrateRendering) {
        dest = curTranform.getTransMatrix();
        return;
    }

    auto& prevTransform = spatials.previousTransforms[meshIdx];

    auto position = glm::mix(prevTransform.getPosition(), curTranform.getPosition(), delta);
    auto rotation = glm::lerp(prevTransform.getRotation(), curTranform.getRotation(), delta);
    auto scale = glm::mix(prevTransform.getScale(), curTranform.getScale(), delta);

    rotation = glm::normalize(rotation);
    dest = glm::translate(glm::mat4(1.0f), position) *
        glm::mat4_cast(rotation) *
        glm::scale(glm::mat4(1.0f), scale);
}

void RenderSystem::drawEntities(RenderFrameContext& ctx, float delta) {
    ZoneScoped;
    //auto view = getEcs().view<Component::Renderable, Component::Spatials, Component::ModelComponent, Component::Material>();

    auto& ecs = getEcs();

    std::vector<entt::entity> entities;
    entities.reserve(64);

    auto* c = static_cast<FreeCameraController*>(cameraController);
    spatialPartitioning->getSpatialGrid()->getVisible(c->getCamera()->getPosition(), c->getFrustumCorners(), c->getFrustumTester(), entities);

    debugCtx->storeIntValue("spatialGridVisibleEntities", entities.size());

    for (auto& e : entities) {
        ZoneScopedN("draw_entity");
        auto& modelComponent = ecs.get<Component::ModelComponent>(e);
        auto modelTask = modelCache->getAsync(modelComponent.config);

        auto& materialComponent = ecs.get<Component::Material>(e);
        auto materialTask = materialCache->getAsync(materialComponent.config);

        // maybe use a default material on a mesh if material isnt ready
        if (!modelTask.isDone())
            continue;

        if (!materialTask.isDone())
            continue;

        auto& renderableComponent = ecs.get<Component::Renderable>(e);

        //if (renderableComponent.type == Component::Renderable::STATIC_MODEL)
        //    continue;

        auto& spatialsComponent = ecs.get<Component::Spatials>(e);

        auto model = modelTask.get();
        auto material = materialTask.get();

        glm::mat4 blendMat{};
        auto curIdx = 0;
        for (const auto& mg : model->getMeshGroups()) {
            for (const auto& m : mg->getMeshes()) {
                ZoneScopedN("draw_entity_mesh");
                auto node = sceneGraph->get(spatialsComponent.meshSpatialsIds[curIdx]);
                auto& curTranform = node->getWorldTransform();
                integrate(renderableComponent, spatialsComponent, curTranform, curIdx, delta, blendMat);

                // TODO: need to make skeleton only up laod once, this might be uploading multiple times if meshes share a skeleton
                // TODO: do this somewhere else, multithread it, and await finish before submitting frame
                if (materialComponent.config->hasSkeleton()) {
                    auto& skeleComp = ecs.get<Component::SkeletonComp>(e);
                    auto skeleton = std::static_pointer_cast<Skeleton>(sceneGraph->get(skeleComp.skeletonId));
                    skeletonManager->upload(*skeleton, skeleComp.bufId, ctx.frameIndex, delta);
                }

                // need to calc in Model still
                auto& bounds = m->getBounds();
                renderCtx->draw(*m, *material, blendMat, bounds.getSphereBounds());

                if (EngineConfig::getInstance().isDebugRenderingEnabled()) {
                    glm::vec3 min, max;
                    m->getBounds().getAabb().getMinMax(min, max);
                    auto scale = max - min;
                    renderCtx->drawDebug(glm::scale(blendMat, scale), glm::vec4{ 1, 0, 0, 1 });
                }

                curIdx++;
            }
        }
    }
}
