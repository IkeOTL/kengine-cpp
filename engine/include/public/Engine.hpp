#pragma once
#include <VulkanContext.hpp>
#include <Window.hpp>
#include <memory>
#include <ExecutorService.hpp>

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