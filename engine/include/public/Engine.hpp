#pragma once
#include <Window.hpp>
#include <memory>
#include <ExecutorService.hpp>
#include <VulkanContext.hpp>

class Engine {

public:
    Engine();
    ~Engine();
    void run();

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<ExecutorService> threadPool;
    std::unique_ptr<VulkanContext> vulkanCxt;

};