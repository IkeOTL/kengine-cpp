#include "Engine.hpp"
#include <iostream>
#include <future>
#include <chrono>
#include <algorithm>

Engine::Engine(VulkanContext::RenderPassCreator rpc, SwapchainCreator::OnSwapchainCreate scc)
    : vulkanCxt(rpc, scc), window("Lolol", 1920, 1080) {
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

    std::thread renderThread([this]() {
        using namespace std::chrono;

        auto lastFrame = high_resolution_clock::now();

        while (!glfwWindowShouldClose(this->window.getWindow())) {
            auto newTime = high_resolution_clock::now();
            delta = duration_cast<nanoseconds>(newTime - lastFrame).count() * .000000001f;
            lastFrame = newTime;

            // cap delta
            delta = std::min(delta, .2f);


        }
        });

    window.awaitEventsLoop();

    renderThread.join();
}

Engine::~Engine() {

}
