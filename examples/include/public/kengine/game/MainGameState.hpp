#pragma once
#include <kengine/StateMachine.hpp>
#include <kengine/game/Game.hpp>

class ExecutorService;
class VulkanContext;
class RenderContext;

class MainGameState : State<Game> {
private:
    ExecutorService& workerPool;
    VulkanContext& vkContext;
    RenderContext& renderContext;

    static constexpr float GAME_UPDATE_TICK_INTERVAL = 1 / 30.0f;
    float accumulator = 0;

public:
    MainGameState(ExecutorService& workerPool, VulkanContext& vkContext, RenderContext& renderContext)
        : workerPool(workerPool), vkContext(vkContext), renderContext(renderContext) {}

    void init() override;
    void enter(Game& parent) override;
    void update(Game& parent) override;
    void exit(Game& parent) override;
};