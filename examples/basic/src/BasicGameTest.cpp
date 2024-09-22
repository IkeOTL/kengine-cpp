#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/Math.hpp>

#include <tracy/Tracy.hpp>
#include <GLFW/glfw3.h>
#include <utility>
#include <kengine/vulkan/pipelines/DebugDeferredOffscreenPbrPipeline.hpp>

#include "RenderablePreviousTransformSystem.hpp"
#include "SpatialGridUpdateSystem.hpp"
#include "RenderSystem.hpp"
#include "BasicGameTest.hpp"
#include "CameraSystem.hpp"

#include "MainGameState.hpp"
#include "Game.hpp"
#include "BasicCameraController.hpp"

float BasicGameTest::getDelta() {
    return delta;
}

void BasicGameTest::run() {
    auto gameState = init();

    sm.setInitialState(gameState.get());
    gameState->enter(*this);

    //std::thread renderThread([this]() {
    using namespace std::chrono;

    auto lastFrame = high_resolution_clock::now();

    while (!glfwWindowShouldClose(this->window->getWindow())) {
        FrameMark;

        auto newTime = high_resolution_clock::now();
        delta = duration_cast<nanoseconds>(newTime - lastFrame).count() * .000000001f;
        lastFrame = newTime;

        // cap delta
        delta = math::min(delta, .2f);

        sm.update();
    }
    //    });

    //renderThread.join();
}

std::unique_ptr<State<Game>> BasicGameTest::init() {
    window = Window::create("rawr", 1920, 1080);
    inputManager = InputManager::create();
    window->setInputManager(inputManager.get());

    initVulkan();

    // review this usage
    threadPool.reset(new ExecutorService(4, [&]() {
        vulkanCxt->getCommandPool()->initThread(*vulkanCxt);
        }));

    debugContext = DebugContext::create();
    initCamera(*inputManager, *debugContext);

    vulkanCxt->setDebugContext(debugContext.get());

    assetIo = FileSystemAssetIO::create();
    lightsManager = LightsManager::create(*cameraController);
    sceneTime = std::make_unique<SceneTime>();
    sceneGraph = SceneGraph::create();
    spatialPartitioningManager = SpatialPartitioningManager::create();
    skeletonManager = SkeletonManager::create(*vulkanCxt);

    spatialPartitioningManager->setSpatialGrid(SpatialGrid::create(64, 64, 16));

    modelFactory = GltfModelFactory::create(*vulkanCxt, *assetIo);

    modelCache = AsyncModelCache::create(*modelFactory, *threadPool);
    animationFactory = GltfAnimationFactory::create(*vulkanCxt, *assetIo);
    animationCache = AsyncAnimationCache::create(*animationFactory, *threadPool);
    textureFactory = TextureFactory::create(*vulkanCxt, *assetIo);
    textureCache = AsyncTextureCache::create(*textureFactory, *threadPool);
    materialCache = AsyncMaterialCache::create(vulkanCxt->getPipelineCache(), *textureCache, vulkanCxt->getGpuBufferCache(), *threadPool);

    imGuiContext = std::make_unique<TestGui>(*vulkanCxt, *sceneTime, *debugContext);
    imGuiContext->init(*window);

    renderContext = RenderContext::create(*vulkanCxt, *lightsManager, *cameraController);
    renderContext->init();
    renderContext->setImGuiContext(imGuiContext.get());

    auto config = AnimationConfig::create("gltf/char01.glb", "Run00");
    auto& lol = animationCache->get(config);

    world = World::create(WorldConfig()
        // injectable objects. order doesnt matter
        .addService<entt::registry>(&ecs)
        .addService<DebugContext>(debugContext.get())
        .addService<Window>(window.get())
        .addService<VulkanContext>(vulkanCxt.get())
        .addService<RenderContext>(renderContext.get())
        .addService<SceneGraph>(sceneGraph.get())
        .addService<SceneTime>(sceneTime.get())
        .addService<SpatialPartitioningManager>(spatialPartitioningManager.get())

        .addService<ExecutorService>(threadPool.get())
        .addService<AssetIO>(assetIo.get())
        .addService<LightsManager>(lightsManager.get())
        .addService<SkeletonManager>(skeletonManager.get())
        .addService<CameraController>(cameraController.get())

        .addService<GltfModelFactory>(modelFactory.get())
        .addService<AsyncModelCache>(modelCache.get())
        .addService<GltfAnimationFactory>(modelFactory.get())
        .addService<AsyncAnimationCache>(modelCache.get())
        .addService<TextureFactory>(textureFactory.get())
        .addService<AsyncTextureCache>(textureCache.get())
        .addService<AsyncMaterialCache>(materialCache.get())

        // systems. order matters.
        .setSystem<RenderablePreviousTransformSystem>()
        .setSystem<CameraSystem>()
        .setSystem<SpatialGridUpdateSystem>()
        .setSystem<RenderSystem>()
    );

    return std::make_unique<MainGameState>(*world);
}

void BasicGameTest::initVulkan() {
    vulkanCxt = VulkanContext::create(
        [](VkDevice vkDevice, ColorFormatAndSpace& cfs) {
            std::vector<std::unique_ptr<RenderPass>> passes;
            passes.emplace_back(DeferredPbrRenderPass::create(vkDevice, cfs));
            passes.emplace_back(CascadeShadowMapRenderPass::create(vkDevice, cfs));
            return passes;
        },
        [](VulkanContext& vkCtx, std::vector<std::unique_ptr<RenderPass>>& rp) {
            auto pc = PipelineCache::create();

            pc->createPipeline<DeferredOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<SkinnedOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<DeferredCompositionPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<CascadeShadowMapPipeline>()
                .init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });

            pc->createPipeline<SkinnedCascadeShadowMapPipeline>()
                .init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });

            pc->createPipeline<DrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

#ifdef KE_DEBUG_RENDER
            pc->createPipeline<DebugDeferredOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
#endif

            return pc;
        },
        [](VulkanContext& vkCxt, Swapchain& swapchain, std::vector<std::unique_ptr<RenderPass>>& renderPasses) {
            auto& rp = renderPasses[0];
            auto sc = vkCxt.getSwapchain();
            rp->createRenderTargets(vkCxt.getVmaAllocator(), sc->getImageViews(), sc->getExtents());
        }
    );

    vulkanCxt->init(*window, true);
}

void BasicGameTest::initCamera(InputManager& inputManager, DebugContext& dbg) {
    auto fov = glm::radians(60.0f);
    auto aspectRatio = (float)window->getWidth() / window->getHeight();
    auto camera = Camera::create(fov, aspectRatio, Camera::NEAR_CLIP, Camera::FAR_CLIP);

    camera->setPosition(glm::vec3(0, 0, 5));

    //glm::quat camRot = camera->getRotation();
    //camRot = glm::rotate(camRot, glm::radians(55.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
    //auto rot = glm::rotate(camRot, glm::radians(190.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //camera->setRotation(rot);

    cameraController = std::make_unique<FreeCameraController>(inputManager);
    cameraController->setCamera(std::move(camera));
}

void TestGui::draw() {
    ImGui::Begin("Another Window");
    ImGui::Text(std::to_string(debugCtx.getIntValue("spatialGridVisibleEntities")).c_str());
    ImGui::End();
}
