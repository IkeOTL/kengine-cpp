#include "Engine.hpp"
#include <iostream>
#include <future>

Engine::Engine(VulkanContext::RenderPassCreator rpc, VulkanContext::OnSwapchainCreate scc)
    : vulkanCxt(VulkanContext(rpc, scc)) {
    threadPool = std::make_unique<ExecutorService>(4);
}

void Engine::run() {
    vulkanCxt.init(window, true);

    //auto future = threadPool->submit([]() {
    //    std::cout << "Task with return value running.\n";
    //    return 42;
    //    });

    //threadPool->execute([]() {
    //    std::cout << "Task without return value running.\n";
    //    });

    //for (int i = 0; i < 100; i++) {
    //    threadPool->execute([i]() {
    //        std::cout << "lol: " << i << "\n";
    //        });
    //}

    //std::cout << "Result from task with return value: " << future.get() << "\n";

    window.pollInput();
}

Engine::~Engine() {

}
