#pragma once
#include <kengine/Game.hpp>
#include <kengine/StateMachine.hpp>

namespace ke {
    class ExecutorService;
    class VulkanContext;
    class RenderContext;
    class World;
    class Window;
} // namespace ke

class MainGameState : public ke::State<ke::Game> {
private:
    ke::World& world;
    ke::Window& window;
    ke::ExecutorService& workerPool;
    ke::VulkanContext& vkContext;
    ke::RenderContext& renderContext;
    ke::SceneTime& sceneTime;

    static constexpr float GAME_UPDATE_TICK_INTERVAL = 1.0f / 60;
    float accumulator = 0;

public:
    MainGameState(ke::World& world);

    void init() override;
    void enter(ke::Game& parent) override;
    void update(ke::Game& parent) override;
    void exit(ke::Game& parent) override;
};