#pragma once
#include <VulkanContext.hpp>
#include <Window.hpp>
#include <memory>
#include <ExecutorService.hpp>

class Engine {

public:
    Engine(VulkanContext::RenderPassCreator rpc);
    ~Engine();
    void run();

private:
    std::unique_ptr<ExecutorService> threadPool;
    Window window;
    VulkanContext vulkanCxt;

};