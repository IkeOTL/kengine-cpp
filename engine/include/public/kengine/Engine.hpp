#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/Window.hpp>
#include <kengine/ExecutorService.hpp>
#include <memory>
#include <kengine/StateMachine.hpp>

class Engine {

public:
    Engine(VulkanContext::RenderPassCreator&& rpc, VulkanContext::PipelineCacheCreator&& pcc, SwapchainCreator::OnSwapchainCreate&& scc);
    ~Engine();
    void run();

    static ExecutorService& getThreadPool() {
        //std::call_once(initThreadPoolFlag, &initThreadPool);
        return *threadPool;
    }

private:
    Window window;
    VulkanContext vulkanCxt;
    float delta;

    static std::unique_ptr<ExecutorService> threadPool;
    //static std::once_flag initThreadPoolFlag;

    static void initThreadPool() {
        threadPool.reset(new ExecutorService(4, []() {}));
    }
};