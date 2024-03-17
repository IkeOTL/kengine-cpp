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
#include <kengine/game/RenderSystem.hpp>
#include <kengine/game/CameraSystem.hpp>


#include <kengine/game/MainGameState.hpp>
#include <kengine/game/Game.hpp>
#include <kengine/math.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <GLFW/glfw3.h>
#include <algorithm>
#include <thread>
#include <utility>
#include <glm/glm.hpp>

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
        auto newTime = high_resolution_clock::now();
        delta = duration_cast<nanoseconds>(newTime - lastFrame).count() * .000000001f;
        lastFrame = newTime;

        // cap delta
        delta = std::min<float>(delta, .2f);

        sm.update();
    }
    //    });

    //renderThread.join();
}

std::unique_ptr<State<Game>> BasicGameTest::init() {
    window = std::make_unique<Window>("rawr", 1920, 1080);
    inputManager = std::make_unique<InputManager>();
    window->setInputManager(inputManager.get());

    initVulkan();
    threadPool.reset(new ExecutorService(4, [&]() {
        vulkanCxt->getCommandPool()->initThread(*vulkanCxt);
        }));

    initCamera(*inputManager);

    assetIo = std::make_unique<FileSystemAssetIO>();
    bufCache = std::make_unique<GpuBufferCache>(*vulkanCxt);
    lightsManager = std::make_unique<LightsManager>(*cameraController);
    sceneTime = std::make_unique<SceneTime>();
    sceneGraph = std::make_unique<SceneGraph>();

    modelFactory = std::make_unique<GltfModelFactory>(*vulkanCxt, *assetIo);
    modelCache = std::make_unique<AsyncModelCache>(*modelFactory, *threadPool);
    textureFactory = std::make_unique<TextureFactory>(*vulkanCxt, *assetIo);
    textureCache = std::make_unique<AsyncTextureCache>(*textureFactory, *threadPool);
    materialCache = std::make_unique<AsyncMaterialCache>(vulkanCxt->getPipelineCache(), *textureCache, *bufCache, *threadPool);

    renderContext = std::make_unique<RenderContext>(*vulkanCxt, *bufCache, *lightsManager, *cameraController);
    renderContext->init();

    world = std::make_unique<World>(WorldConfig()
        // injectable objects. order doesnt matter
        .addService<Window>(window.get())
        .addService<VulkanContext>(vulkanCxt.get())
        .addService<RenderContext>(renderContext.get())
        .addService<SceneGraph>(sceneGraph.get())
        .addService<SceneTime>(sceneTime.get())
        .addService<entt::registry>(&ecs)

        .addService<ExecutorService>(threadPool.get())
        .addService<AssetIO>(assetIo.get())
        .addService<LightsManager>(lightsManager.get())
        .addService<CameraController>(cameraController.get())

        .addService<GpuBufferCache>(bufCache.get())
        .addService<GltfModelFactory>(modelFactory.get())
        .addService<AsyncModelCache>(modelCache.get())
        .addService<TextureFactory>(textureFactory.get())
        .addService<AsyncTextureCache>(textureCache.get())
        .addService<AsyncMaterialCache>(materialCache.get())

        // systems. order matters.
        .setSystem<RenderSystem>()
        .setSystem<CameraSystem>()
    );

    return std::make_unique<MainGameState>(*world);
}

void BasicGameTest::initVulkan() {
    vulkanCxt = std::make_unique<VulkanContext>(
        [](VkDevice vkDevice, ColorFormatAndSpace& cfs) {
            std::vector<std::unique_ptr<RenderPass>> passes;
            passes.push_back(std::move(std::make_unique<DeferredPbrRenderPass>(vkDevice, cfs)));
            passes.push_back(std::move(std::make_unique<CascadeShadowMapRenderPass>(vkDevice, cfs)));
            return passes;
        },
        [](VulkanContext& vkCtx, std::vector<std::unique_ptr<RenderPass>>& rp) {
            auto pc = std::make_unique<PipelineCache>();

            auto pass0 = std::make_unique<DeferredOffscreenPbrPipeline>();
            pass0->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(pass0));

            auto skinned = std::make_unique<SkinnedOffscreenPbrPipeline>();
            skinned->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(skinned));

            auto pass1 = std::make_unique<DeferredCompositionPbrPipeline>();
            pass1->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(pass1));

            auto shadowPass = std::make_unique<CascadeShadowMapPipeline>();
            shadowPass->init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });
            pc->addPipeline(std::move(shadowPass));

            auto skinnedShadowPass = std::make_unique<SkinnedCascadeShadowMapPipeline>();
            skinnedShadowPass->init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });
            pc->addPipeline(std::move(skinnedShadowPass));

            auto culling = std::make_unique<DrawCullingPipeline>();
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

void BasicGameTest::initCamera(InputManager& inputManager) {
    auto fov = glm::radians(60.0f);
    auto aspectRatio = (float)window->getWidth() / window->getHeight();
    auto camera = std::make_unique<Camera>(fov, aspectRatio, Camera::NEAR_CLIP, Camera::FAR_CLIP);

    camera->setPosition(glm::vec3(0, 0, 5));

    //glm::quat camRot = camera->getRotation();
    //camRot = glm::rotate(camRot, glm::radians(55.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
    //auto rot = glm::rotate(camRot, glm::radians(190.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //camera->setRotation(rot);

    cameraController = std::make_unique<FreeCameraController>(inputManager);
    cameraController->setCamera(std::move(camera));
}
