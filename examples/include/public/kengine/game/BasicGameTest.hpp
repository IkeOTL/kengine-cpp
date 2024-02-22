#pragma once
#include <kengine/game/Game.hpp>
#include <kengine/game/MainGameState.hpp>
#include <kengine/StateMachine.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/Window.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/io/AssetIO.hpp>
#include <memory>

class BasicGameTest : Game {
private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanCxt;
    std::unique_ptr<RenderContext> renderContext;
    std::unique_ptr<GpuBufferCache> bufCache;
    std::unique_ptr<FileSystemAssetIO> assetIo;
    std::unique_ptr<LightsManager> lightsManager;
    std::unique_ptr<CameraController> cameraController;

    StateMachine<Game> sm;
    inline static std::unique_ptr<ExecutorService> threadPool;

    float delta = 0;

    std::unique_ptr<State<Game>> init();
    void initVulkan();
    void initCamera();

public:
    BasicGameTest() : sm(*this, nullptr, nullptr, nullptr) {}

    float getDelta() override;
    void run() override;
};