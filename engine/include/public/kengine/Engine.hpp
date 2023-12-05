#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/Window.hpp>
#include <kengine/ExecutorService.hpp>
#include <memory>
#include <kengine/StateMachine.hpp>

class Engine {

public:
    Engine(VulkanContext::RenderPassCreator rpc, SwapchainCreator::OnSwapchainCreate scc);
    ~Engine();
    void run();

private:
    ExecutorService threadPool;
    Window window;
    VulkanContext vulkanCxt;
    float delta;

};