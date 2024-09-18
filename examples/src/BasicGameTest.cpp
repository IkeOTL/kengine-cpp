#define NOMINMAX

#include <kengine/game/BasicGameTest.hpp>
#include <kengine/game/BasicCameraController.hpp>
#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/game/RenderablePreviousTransformSystem.hpp>
#include <kengine/game/SpatialGridUpdateSystem.hpp>
#include <kengine/game/RenderSystem.hpp>
#include <kengine/game/CameraSystem.hpp>

#include <kengine/game/MainGameState.hpp>
#include <kengine/game/Game.hpp>
#include <kengine/Math.hpp>

#include <tracy/Tracy.hpp>
#include <GLFW/glfw3.h>
#include <utility>

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

    debugContext = std::make_unique<DebugContext>();
    initCamera(*inputManager, *debugContext);

    vulkanCxt->setDebugContext(debugContext.get());

    assetIo = std::make_unique<FileSystemAssetIO>();
    bufCache = std::make_unique<GpuBufferCache>(*vulkanCxt);
    lightsManager = std::make_unique<LightsManager>(*cameraController);
    sceneTime = std::make_unique<SceneTime>();
    sceneGraph = std::make_unique<SceneGraph>();
    spatialPartitioningManager = std::make_unique<SpatialPartitioningManager>();
    skeletonManager = std::make_unique<SkeletonManager>(*vulkanCxt, *bufCache);

    spatialPartitioningManager->setSpatialGrid(std::make_unique<SpatialGrid>(64, 64, 16));

    modelFactory = std::make_unique<GltfModelFactory>(*vulkanCxt, *assetIo);
    modelCache = std::make_unique<AsyncModelCache>(*modelFactory, *threadPool);
    animationFactory = std::make_unique<GltfAnimationFactory>(*vulkanCxt, *assetIo);
    animationCache = std::make_unique<AsyncAnimationCache>(*animationFactory, *threadPool);
    textureFactory = std::make_unique<TextureFactory>(*vulkanCxt, *assetIo);
    textureCache = std::make_unique<AsyncTextureCache>(*textureFactory, *threadPool);
    materialCache = std::make_unique<AsyncMaterialCache>(vulkanCxt->getPipelineCache(), *textureCache, *bufCache, *threadPool);

    imGuiContext = std::make_unique<TestGui>(*vulkanCxt, *sceneTime, *debugContext);
    imGuiContext->init(*window);

    renderContext = std::make_unique<RenderContext>(*vulkanCxt, *bufCache, *lightsManager, *cameraController);
    renderContext->init();
    renderContext->setImGuiContext(imGuiContext.get());

    auto config = std::make_shared<AnimationConfig>("res/gltf/char01.glb", "Run00");
    auto& lol = animationCache->get(config);


    world = std::make_unique<World>(WorldConfig()
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

        .addService<GpuBufferCache>(bufCache.get())
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

            auto pass0 = DeferredOffscreenPbrPipeline::create();
            pass0->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(pass0));

            auto skinned = SkinnedOffscreenPbrPipeline::create();
            skinned->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(skinned));

            auto pass1 = DeferredCompositionPbrPipeline::create();
            pass1->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(pass1));

            auto shadowPass = CascadeShadowMapPipeline::create();
            shadowPass->init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });
            pc->addPipeline(std::move(shadowPass));

            auto skinnedShadowPass = SkinnedCascadeShadowMapPipeline::create();
            skinnedShadowPass->init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });
            pc->addPipeline(std::move(skinnedShadowPass));

            auto culling = DrawCullingPipeline::create();
            culling->init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(culling));

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
