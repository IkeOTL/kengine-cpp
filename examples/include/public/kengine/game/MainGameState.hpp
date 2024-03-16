#pragma once
#include <kengine/StateMachine.hpp>
#include <kengine/game/Game.hpp>

class ExecutorService;
class VulkanContext;
class RenderContext;
class World;
class Window;

class MainGameState : public State<Game> {
private:
    World& world;
    Window& window;
    ExecutorService& workerPool;
    VulkanContext& vkContext;
    RenderContext& renderContext;
    SceneTime& sceneTime;

    static constexpr float GAME_UPDATE_TICK_INTERVAL = 1 / 30.0f;
    float accumulator = 0;

public:
    MainGameState(World& world);

    void init() override;
    void enter(Game& parent) override;
    void update(Game& parent) override;
    void exit(Game& parent) override;
};