
#include "RenderSystem.hpp"
#include <kengine/Game.hpp>
#include "components/Material.hpp"
#include "components/Model.hpp"
#include "components/Components.hpp"
#include "BasicCameraController.hpp"

#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/DebugContext.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/SpatialPartitioningManager.hpp>
#include <kengine/terrain/TileTerrain.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>
#include <kengine/util/Random.hpp>
#include <kengine/vulkan/SkeletonManager.hpp>
#include <tracy/Tracy.hpp>
#include <kengine/util/MatUtils.hpp>
#include <kengine/EngineConfig.hpp>

void RenderSystem::init() {
    vulkanCtx = getService<ke::VulkanContext>();
    debugCtx = getService<ke::DebugContext>();
    renderCtx = getService<ke::RenderContext>();
    modelCache = getService<ke::AsyncModelCache>();
    materialCache = getService<ke::AsyncMaterialCache>();
    sceneGraph = getService<ke::SceneGraph>();
    sceneTime = getService<ke::SceneTime>();
    cameraController = getService<ke::CameraController>();
    spatialPartitioning = getService<ke::SpatialPartitioningManager>();
    skeletonManager = getService<ke::SkeletonManager>();
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

void RenderSystem::integrate(Component::Renderable& renderable, Component::Spatials& spatials,
    ke::Transform& curTranform, uint32_t meshIdx, float delta, glm::mat4& dest) {
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

void RenderSystem::drawEntities(ke::RenderFrameContext& ctx, float delta) {
    ZoneScoped;

    auto& ecs = getEcs();

    auto* c = static_cast<FreeCameraController*>(cameraController);
    auto visibleCount = spatialPartitioning->getSpatialGrid()->getVisible(
        c->getCamera()->getPosition(),
        c->getFrustumCorners(),
        c->getFrustumTester(),
        visibleEntities,
        maxVisibleEntities
    );

    debugCtx->storeIntValue("spatialGridVisibleEntities", visibleCount);

    for (auto eIdx = 0; eIdx < visibleCount; eIdx++) {
        ZoneScopedN("draw_entity");

        auto& e = visibleEntities[eIdx];

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

        // TODO: need to make skeleton only up laod once, this might be uploading multiple times if meshes share a skeleton
        // TODO: do this somewhere else, multithread it, and await finish before submitting frame
        if (materialComponent.config->hasSkeleton()) {
            auto& skeleComp = ecs.get<Component::SkeletonComp>(e);
            auto skeleton = std::static_pointer_cast<ke::Skeleton>(sceneGraph->get(skeleComp.skeletonId));
            skeletonManager->upload(*skeleton, skeleComp.bufId, ctx.frameIndex, delta);
        }

        auto* model = modelTask.get();
        auto* material = materialTask.get();

        glm::mat4 blendMat{};
        auto curIdx = 0;
        for (const auto& mg : model->getMeshGroups()) {
            for (const auto& m : mg->getMeshes()) {
                ZoneScopedN("draw_entity_mesh");
                auto node = sceneGraph->get(spatialsComponent.meshSpatialsIds[curIdx]);
                auto& curTranform = node->getWorldTransform();
                integrate(renderableComponent, spatialsComponent, curTranform, curIdx, delta, blendMat);

                // need to calc in Model still
                auto& bounds = m->getBounds();
                // todo: spherebounds need to scale based on spatial!!
                renderCtx->draw(*m, *material, blendMat, bounds.getSphereBounds());

                if (ke::EngineConfig::getInstance().isDebugRenderingEnabled()) {
                    auto& aabb = m->getBounds().getAabb();
                    glm::vec3 min, max;
                    aabb.getMinMax(min, max);

                    auto debugMat4 = glm::translate(blendMat, aabb.pos);

                    auto scale = max - min;
                    renderCtx->drawDebug(glm::scale(debugMat4, scale), glm::vec4{ 1, 0, 0, 1 });
                }

                curIdx++;
            }
        }
    }
}
