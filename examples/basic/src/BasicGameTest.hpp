#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/GltfModelFactory.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/SpatialPartitioningManager.hpp>
#include <kengine/DebugContext.hpp>
#include <kengine/StateMachine.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/mesh/anim/GltfAnimationFactory.hpp>
#include <kengine/vulkan/mesh/anim/AsyncAnimationCache.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/vulkan/SkeletonManager.hpp>
#include <kengine/vulkan/ImGuiKEContext.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/Window.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/io/AssetIO.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>

#include <memory>
#include "Game.hpp"

class TestGui : public ImGuiKEContext {
private:
    DebugContext& debugCtx;
    SceneTime& sceneTime;

public:
    TestGui(VulkanContext& vkCtx, SceneTime& sceneTime, DebugContext& debugCtx)
        : ImGuiKEContext(vkCtx), sceneTime(sceneTime), debugCtx(debugCtx) {}

protected:
    void draw() override;
};

class BasicGameTest : Game {
private:
    inline static std::unique_ptr<ExecutorService> threadPool;

    std::unique_ptr<Window> window;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<DebugContext> debugContext;
    std::unique_ptr<VulkanContext> vulkanCxt;
    std::unique_ptr<ImGuiKEContext> imGuiContext;
    std::unique_ptr<TerrainContext> terrainContext;
    std::unique_ptr<RenderContext> renderContext;
    std::unique_ptr<AssetIO> assetIo;
    std::unique_ptr<SceneGraph> sceneGraph;
    std::unique_ptr<LightsManager> lightsManager;
    std::unique_ptr<SkeletonManager> skeletonManager;
    std::unique_ptr<CameraController> cameraController;
    std::unique_ptr<SceneTime> sceneTime;
    std::unique_ptr<SpatialPartitioningManager> spatialPartitioningManager;

    std::unique_ptr<GltfModelFactory> modelFactory;
    std::unique_ptr<AsyncModelCache> modelCache;
    std::unique_ptr<GltfAnimationFactory> animationFactory;
    std::unique_ptr<AsyncAnimationCache> animationCache;
    std::unique_ptr<TextureFactory> textureFactory;
    std::unique_ptr<AsyncTextureCache> textureCache;
    std::unique_ptr<AsyncMaterialCache> materialCache;

    entt::registry ecs;
    std::unique_ptr<World> world;
    StateMachine<Game> sm;

    float delta = 0;
    const bool isDebugRendering = true;

    std::unique_ptr<State<Game>> init();
    void initVulkan();
    void initCamera(InputManager& inputManager, DebugContext& dbgCtx);


public:
    BasicGameTest() : sm(*this, nullptr, nullptr, nullptr) {}

    float getDelta() override;
    void run() override;
};