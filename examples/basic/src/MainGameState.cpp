#include "MainGameState.hpp"
#include <kengine/Game.hpp>
#include "RenderSystem.hpp"
#include "RenderablePreviousTransformSystem.hpp"

#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/Window.hpp>
#include <kengine/ecs/World.hpp>
#include <kengine/vulkan/mesh/ModelConfig.hpp>

MainGameState::MainGameState(World& world) :
    world(world),
    window(*world.getService<Window>()),
    workerPool(*world.getService<ExecutorService>()),
    vkContext(*world.getService<VulkanContext>()),
    renderContext(*world.getService<RenderContext>()),
    sceneTime(*world.getService<SceneTime>()) {}

void MainGameState::init() {
}

void MainGameState::enter(Game& parent) {
    glfwShowWindow(window.getWindow());
}

//// save for later so i dont forget how to hook up skeletons
//void createPlayerEntity() {
//    auto* ecs = getWorld().getService<entt::registry>();
//    auto modelConfig = std::make_shared<ModelConfig>("res/gltf/char01.glb",
//        VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
//        | VertexAttribute::TANGENTS | VertexAttribute::SKELETON
//    );
//
//    auto entity = ecs->create();
//    auto& renderable = ecs->emplace<Component::Renderable>(entity);
//    //renderable.setStatic();
//    ecs->emplace<Component::ModelComponent>(entity, modelConfig);
//
//
//
//    auto& model = modelCache->get(modelConfig);
//    auto& spatials = ecs->emplace<Component::Spatials>(entity);
//    auto rootSpatial = spatials.generate(*sceneGraph, model, "player" + std::to_string(i), renderable.type);
//
//    //rootSpatial->setChangeCb(spatialPartitioning->getSpatialGrid()->createCb(entity));
//
//    rootSpatial->setLocalPosition(0, 2, 5);
//
//    spatialPartitioning->getSpatialGrid()->setDirty(entity);
//
//    auto& skeletonComp = ecs->emplace<Component::SkeletonComp>(entity);
//    auto skeleton = skeletonComp.generate(*sceneGraph, model, spatials, "LOL SKELE");
//    auto skeletonId = sceneGraph->add(skeleton);
//    skeletonComp.skeletonId = skeletonId;
//    auto& buf = skeletonManager->createMappedBuf(*skeleton);
//    skeletonComp.bufId = buf.getId();
//    // review if parenting the skeleton thats made of already parented nodes is an issue
//    rootSpatial->addChild(skeleton);
//
//    auto materialConfig = std::make_shared<PbrMaterialConfig>(skeletonComp.bufId);
//    materialConfig->setHasShadow(true);
//    ecs->emplace<Component::Material>(entity, materialConfig);
//}

void MainGameState::update(Game& parent) {
    window.pollEvents();

    auto delta = parent.getDelta();

    accumulator += delta;

    while (accumulator >= GAME_UPDATE_TICK_INTERVAL) {
        sceneTime.setDelta(GAME_UPDATE_TICK_INTERVAL);

        // these only need to be updated as the last updates
        // typically used for alpha lerping
        if (accumulator - GAME_UPDATE_TICK_INTERVAL < GAME_UPDATE_TICK_INTERVAL) {
            world.getSystem<RenderablePreviousTransformSystem>()->processSystem(GAME_UPDATE_TICK_INTERVAL);
            //world.getSystem<SkeletonPreviousTransformSystem>()->processSystem(GAME_UPDATE_TICK_INTERVAL);
        }

        world.process(GAME_UPDATE_TICK_INTERVAL);
        accumulator -= GAME_UPDATE_TICK_INTERVAL;
        sceneTime.addSceneTime(GAME_UPDATE_TICK_INTERVAL);
    }

    auto alpha = accumulator / GAME_UPDATE_TICK_INTERVAL;
    sceneTime.setAlpha(alpha);
    world.getSystem<RenderSystem>()->processSystem(alpha);
}

void MainGameState::exit(Game& parent) {
}
