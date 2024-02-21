#pragma once
#include <kengine/game/Game.hpp>
#include <kengine/StateMachine.hpp>
#include <memory>

class Window;
class VulkanContext;
class ExecutorService;
class GpuBufferCache;
class FileSystemAssetIO;
class LightManager;
class CameraController;

class BasicGameTest : Game {
private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanCxt;
    std::unique_ptr<GpuBufferCache> bufCache;
    std::unique_ptr<FileSystemAssetIO> assetIo;
    std::unique_ptr<LightManager> lightsManager;
    std::unique_ptr<CameraController> cameraController;

    StateMachine<BasicGameTest> sm;
    inline static std::unique_ptr<ExecutorService> threadPool;

    float delta;

    std::unique_ptr<State<BasicGameTest>> init();
    void initVulkan();
    void initCamera();

public:
    BasicGameTest() : sm(*this, nullptr, nullptr, nullptr) {}

    float getDelta() override;
    void run() override;
};