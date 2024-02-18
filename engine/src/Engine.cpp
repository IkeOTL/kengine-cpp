#include <kengine/Engine.hpp>

#include <iostream>
#include <future>
#include <chrono>
#include <algorithm>

std::unique_ptr<ExecutorService> Engine::threadPool;
//std::once_flag Engine::initThreadPoolFlag;

Engine::Engine(VulkanContext::RenderPassCreator&& rpc, VulkanContext::PipelineCacheCreator&& pcc, SwapchainCreator::OnSwapchainCreate&& scc)
    : vulkanCxt(std::move(rpc), std::move(pcc), std::move(scc)), window("Lolol", 1920, 1080) {}

void Engine::run() {
    initThreadPool();
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

        StateMachine<Engine> sm(*this, nullptr, nullptr, nullptr);

        auto lastFrame = high_resolution_clock::now();

        while (!glfwWindowShouldClose(this->window.getWindow())) {
            auto newTime = high_resolution_clock::now();
            delta = duration_cast<nanoseconds>(newTime - lastFrame).count() * .000000001f;
            lastFrame = newTime;

            // cap delta
            delta = std::min(delta, .2f);

            sm.update();
        }
        });

    window.awaitEventsLoop();

    renderThread.join();
}

Engine::~Engine() {

}
