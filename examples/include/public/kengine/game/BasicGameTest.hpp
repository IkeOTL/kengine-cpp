#pragma once
#include <kengine/game/Game.hpp>
#include <kengine/StateMachine.hpp>
#include <memory>

class Window;
class VulkanContext;
class ExecutorService;

class BasicGameTest : Game {
private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanCxt;

    StateMachine<BasicGameTest> sm;
    inline static std::unique_ptr<ExecutorService> threadPool;

    float delta;

    std::unique_ptr<State<BasicGameTest>> init();
    void initVulkan();

public:
    BasicGameTest() : sm(*this, nullptr, nullptr, nullptr) {}

    float getDelta() override;
    void run() override;
};