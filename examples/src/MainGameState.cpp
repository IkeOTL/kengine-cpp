#include <kengine/game/MainGameState.hpp>

void MainGameState::init() {

}

void MainGameState::enter(Game& parent) {
}

void MainGameState::update(Game& parent) {
    auto delta = parent.getDelta();

    accumulator += delta;

    while (accumulator >= GAME_UPDATE_TICK_INTERVAL) {
        // these only need to be updated as the last updates
        // typically used for alpha lerping
        if (accumulator - GAME_UPDATE_TICK_INTERVAL < GAME_UPDATE_TICK_INTERVAL) {
            //world.getSystem(RenderablePreviousTransformSystem.class).processSystem();
            //world.getSystem(SkeletonPreviousTransformSystem.class).processSystem();
        }

        //sceneTime.setDelta(GAME_UPDATE_TICK_INTERVAL);
        //world.setDelta(GAME_UPDATE_TICK_INTERVAL);
        //world.process();
        accumulator -= GAME_UPDATE_TICK_INTERVAL;
    }

    //sceneTime.addSceneTime(delta);

    auto alpha = accumulator / GAME_UPDATE_TICK_INTERVAL;
    //sceneTime.setAlpha(alpha);
    //world.setDelta(alpha);
    //world.getSystem(DeferredRenderSystem.class).processSystem();

}

void MainGameState::exit(Game& parent) {
}
