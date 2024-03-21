#include <kengine/game/MainGameState.hpp>
#include <kengine/game/Game.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/game/RenderablePreviousTransformSystem.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/Window.hpp>
#include <kengine/ecs/World.hpp>
#include <kengine/game/RenderSystem.hpp>
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
            //world.getSystem(SkeletonPreviousTransformSystem.class).processSystem();
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
