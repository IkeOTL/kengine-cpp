#pragma once
#include <Jolt/Jolt.h>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/GltfModelFactory.hpp>
#include <kengine/vulkan/mesh/AsyncModelCache.hpp>
#include <kengine/SpatialPartitioningManager.hpp>
#include <kengine/DebugContext.hpp>
#include <kengine/StateMachine.hpp>
#include "BasicCameraController.hpp"
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
#include <kengine/Game.hpp>
#include <kengine/DeferredJob.hpp>
#include <kengine/EventBus.hpp>
#include <PlayerMovementManager.hpp>
#include "MyPlayerContext.hpp"
#include <PhysicsContext.hpp>

class TestGui : public ke::ImGuiKEContext, public ke::WorldService {
private:
    ke::DebugContext& debugCtx;
    ke::SceneTime& sceneTime;

public:
    TestGui(ke::VulkanContext& vkCtx, ke::SceneTime& sceneTime, ke::DebugContext& debugCtx)
        : ke::ImGuiKEContext(vkCtx),
          sceneTime(sceneTime),
          debugCtx(debugCtx) {}

protected:
    void draw() override;
};

class BasicGameTest : ke::Game {
private:
    inline static std::unique_ptr<ke::ExecutorService> threadPool;

    std::unique_ptr<ke::VulkanContext> vulkanCxt;
    std::unique_ptr<ke::Window> window;
    std::unique_ptr<ke::InputManager> inputManager;

    std::unique_ptr<ke::AssetIO> assetIo;
    std::unique_ptr<ke::GltfModelFactory> modelFactory;
    std::unique_ptr<ke::AsyncModelCache> modelCache;
    std::unique_ptr<ke::GltfAnimationFactory> animationFactory;
    std::unique_ptr<ke::AsyncAnimationCache> animationCache;
    std::unique_ptr<ke::TextureFactory> textureFactory;
    std::unique_ptr<ke::AsyncTextureCache> textureCache;
    std::unique_ptr<ke::AsyncMaterialCache> materialCache;

    std::unique_ptr<ke::DebugContext> debugContext;
    std::unique_ptr<ke::EventBus> eventBus;
    std::unique_ptr<ke::DeferredJobManager> djm;
    std::unique_ptr<TestGui> imGuiContext;
    std::unique_ptr<ke::TerrainContext> terrainContext;
    std::unique_ptr<ke::RenderContext> renderContext;
    std::unique_ptr<ke::SceneGraph> sceneGraph;
    std::unique_ptr<ke::LightsManager> lightsManager;
    std::unique_ptr<ke::SkeletonManager> skeletonManager;
    std::unique_ptr<ke::CameraController> cameraController;
    std::unique_ptr<ke::SceneTime> sceneTime;
    std::unique_ptr<ke::SpatialPartitioningManager> spatialPartitioningManager;
    std::unique_ptr<MyPlayerContext> myPlayerContext;
    std::unique_ptr<PlayerMovementManager> playerMovementManager;
    std::unique_ptr<PhysicsContext> physicsContext;

    entt::registry ecs;
    std::unique_ptr<ke::World> world;
    ke::StateMachine<Game> sm;

    float delta = 0;

    std::unique_ptr<ke::State<ke::Game>> init();
    void initVulkan(ke::Window& window);
    void initCamera(ke::InputManager& inputManager, ke::DebugContext& dbgCtx);

public:
    BasicGameTest()
        : sm(*this, nullptr, nullptr, nullptr) {}
    ~BasicGameTest();

    float getDelta() override;
    void run() override;
};