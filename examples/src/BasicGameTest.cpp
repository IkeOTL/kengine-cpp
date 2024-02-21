#include <kengine/game/BasicGameTest.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/ExecutorService.hpp>
#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include<kengine/game/MainGameState.hpp>
#include <kengine/Window.hpp>
#include <kengine/math.hpp>
#include <kengine/io/AssetIO.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/vulkan/Camera.hpp>
#include <kengine/vulkan/CameraController.hpp>

#include <GLFW/glfw3.h>
#include <algorithm>
#include <thread>
#include <utility>
#include <glm/glm.hpp>

float BasicGameTest::getDelta() {
    return 0.0f;
}

void BasicGameTest::run() {
    auto gameState = init();

    sm.setInitialState(gameState.get());

    std::thread renderThread([this]() {
        using namespace std::chrono;

        auto lastFrame = high_resolution_clock::now();

        while (!glfwWindowShouldClose(this->window->getWindow())) {
            auto newTime = high_resolution_clock::now();
            delta = duration_cast<nanoseconds>(newTime - lastFrame).count() * .000000001f;
            lastFrame = newTime;

            // cap delta
            delta = std::min(delta, .2f);

            sm.update();
        }
        });

    window->awaitEventsLoop();

    renderThread.join();
}

std::unique_ptr<State<BasicGameTest>> BasicGameTest::init() {

    window = std::make_unique<Window>("rawr", 1920, 1080);
    initVulkan();
    threadPool.reset(new ExecutorService(4, [&]() {
        vulkanCxt->getCommandPool()->initThread(*vulkanCxt);
        }));

    assetIo = std::make_unique<FileSystemAssetIO>();
    bufCache = std::make_unique<GpuBufferCache>(*vulkanCxt);
    lightsManager = std::make_unique<LightManager>(*vulkanCxt);

    auto gameState = std::make_unique<MainGameState>(*threadPool, *window, *vulkanCxt);

    return std::unique_ptr<State<BasicGameTest>>();
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

void BasicGameTest::initCamera() {
    auto fov = (float)Math.toRadians(60f);
    auto aspectRatio = (float)window.getWidth() / window.getHeight();
    auto camera = new Camera(fov, aspectRatio, Camera.NEAR, Camera.FAR);
    camera.getPosition().set(6, 10, 6);
    camera.getRotation()
        .rotateAxis((float)Math.toRadians(55), new Vector3f(1, 0, 0))
        .rotateAxis((float)Math.toRadians(-45), new Vector3f(0, 1, 0));

    cameraController = std::make_unique<CameraController>();
    cameraController->setCamer
}
