#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
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
#include <kengine/Game.hpp>
#include "BasicCameraController.hpp"
#include <kengine/Logger.hpp>
#include <kengine/EngineConfig.hpp>
#include <kengine/vulkan/pipelines/PreDrawCullingPipeline.hpp>
#include <kengine/vulkan/pipelines/TerrainDrawCullingPipeline.hpp>
#include <kengine/vulkan/pipelines/TerrainDeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/TerrainPreDrawCullingPipeline.hpp>


#include "components/Components.hpp"
#include <components/Material.hpp>
#include <components/Model.hpp>
#include <kengine/util/Random.hpp>
#include <KinematicPlayerSystem.hpp>

#include <PhysicsSystem.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Math/Math.h>
#include <components/Physics.hpp>
#include <PhysicsSyncSystem.hpp>


BasicGameTest::~BasicGameTest() {
   // textureCache.reset();
}

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

        if (inputManager->isKeyDown(GLFW_KEY_ESCAPE)) {
            vkDeviceWaitIdle(vulkanCxt->getVkDevice());
            break;
        }

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

    djm = DeferredJobManager::create();

    assetIo = FileSystemAssetIO::create();
    lightsManager = LightsManager::create(*cameraController);
    sceneTime = std::make_unique<SceneTime>();
    sceneGraph = SceneGraph::create();
    spatialPartitioningManager = SpatialPartitioningManager::create();
    skeletonManager = SkeletonManager::create(*vulkanCxt);

    eventBus = EventBus::create(*sceneTime);
    //eventBus->registerHandler([](const auto& evt, auto& w) {});

    spatialPartitioningManager->setSpatialGrid(SpatialGrid::create(64, 64, 16));

    modelFactory = GltfModelFactory::create(*vulkanCxt, *assetIo);

    modelCache = AsyncModelCache::create(*modelFactory, *threadPool);
    animationFactory = GltfAnimationFactory::create(*vulkanCxt, *assetIo);
    animationCache = AsyncAnimationCache::create(*animationFactory, *threadPool);
    textureFactory = TextureFactory::create(*vulkanCxt, *assetIo);
    textureCache = AsyncTextureCache::create(*textureFactory, *threadPool);
    materialCache = AsyncMaterialCache::create(vulkanCxt->getPipelineCache(), *textureCache, vulkanCxt->getGpuBufferCache(), *threadPool);

    myPlayerContext = MyPlayerContext::create();
    playerMovementManager = PlayerMovementManager::create();

    imGuiContext = std::make_unique<TestGui>(*vulkanCxt, *sceneTime, *debugContext);
    imGuiContext->init(*window);

    terrainContext = std::make_unique<TerrainContext>(*materialCache);

    renderContext = RenderContext::create(*vulkanCxt, *lightsManager, *cameraController);
    renderContext->init(terrainContext.get());
    renderContext->setImGuiContext(imGuiContext.get());

    physicsContext = PhysicsContext::create();
    physicsContext->init();

    world = World::create(WorldConfig()
        // injectable objects. order doesnt matter
        .addService(&ecs)
        .addService(debugContext.get())
        .addService(imGuiContext.get())
        .addService(window.get())
        .addService(vulkanCxt.get())
        .addService(renderContext.get())
        .addService(sceneGraph.get())
        .addService(sceneTime.get())
        .addService(spatialPartitioningManager.get())
        .addService(eventBus.get())
        .addService(playerMovementManager.get())
        .addService(myPlayerContext.get())
        .addService(inputManager.get())
        .addService(physicsContext.get())

        .addService(threadPool.get())
        .addService(assetIo.get())
        .addService(lightsManager.get())
        .addService(skeletonManager.get())
        .addService(cameraController.get())

        .addService(modelFactory.get())
        .addService(modelCache.get())
        .addService(textureFactory.get())
        .addService(textureCache.get())
        .addService(materialCache.get())

        // systems. order matters.
        .setSystem<RenderablePreviousTransformSystem>()
        .setSystem<KinematicPlayerSystem>()
        .setSystem<PhysicsSystem>()
        .setSystem<PhysicsSyncSystem>()
        .setSystem<CameraSystem>()
        .setSystem<SpatialGridUpdateSystem>()
        .setSystem<RenderSystem>()
    );

    // pause physics by defaut
    world->getSystem<PhysicsSystem>()->setPaused(true);

    // player dummy
    /*{
        auto* ecs = world->getService<entt::registry>();
        auto modelConfig = ModelConfig::create("gltf/smallcube.glb",
            VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
            | VertexAttribute::TANGENTS
        );

        auto materialConfig = PbrMaterialConfig::create();
        materialConfig->setHasShadow(true);

        auto entity = ecs->create();
        myPlayerContext->setPlayerEntityId(entity);

        auto& renderable = ecs->emplace<Component::Renderable>(entity);
        auto& spatials = ecs->emplace<Component::Spatials>(entity);
        ecs->emplace<Component::ModelComponent>(entity, modelConfig);
        ecs->emplace<Component::Material>(entity, materialConfig);
        ecs->emplace<Component::LinearVelocity>(entity);

        auto& model = modelCache->get(modelConfig);
        auto rootSpatial = spatials.generate(*sceneGraph, model, "player", renderable.type);
        rootSpatial->setChangeCb(spatialPartitioningManager->getSpatialGrid()->createCb(entity));
        rootSpatial->setLocalPosition(glm::vec3(0, 2, 0));
        rootSpatial->setLocalScale(glm::vec3(5, .5f, 5));
    }*/

    // physics experiment
    {
        // static platform
        {
            glm::vec3 pos(0, 2, 0);
            glm::vec3 size(15, .5f, 15);

            // physics
            JPH::BodyInterface& bodyInterface = physicsContext->getPhysics().GetBodyInterface();
            JPH::BoxShapeSettings shapeSettings(JPH::Vec3(.5f, .5f, .5f) * JPH::Vec3(size.x, size.y, size.z));
            shapeSettings.SetEmbedded();
            JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
            auto& shape = shapeResult.Get();
            JPH::BodyCreationSettings bodySettings(shape, JPH::RVec3(pos.x, pos.y, pos.z), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
            JPH::Body* body = bodyInterface.CreateBody(bodySettings);
            bodyInterface.AddBody(body->GetID(), JPH::EActivation::DontActivate);

            //entity
            auto* ecs = world->getService<entt::registry>();
            auto modelConfig = ModelConfig::create("gltf/smallcube.glb",
                VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
                | VertexAttribute::TANGENTS
            );

            auto materialConfig = PbrMaterialConfig::create();
            materialConfig->setHasShadow(true);

            auto entity = ecs->create();

            auto& renderable = ecs->emplace<Component::Renderable>(entity);
            auto& spatials = ecs->emplace<Component::Spatials>(entity);
            ecs->emplace<Component::Rigidbody>(entity, body->GetID(), false);
            ecs->emplace<Component::ModelComponent>(entity, modelConfig);
            ecs->emplace<Component::Material>(entity, materialConfig);
            //ecs->emplace<Component::LinearVelocity>(entity);

            auto& model = modelCache->get(modelConfig);
            auto rootSpatial = spatials.generate(*sceneGraph, model, "platform", renderable.type);
            rootSpatial->setChangeCb(spatialPartitioningManager->getSpatialGrid()->createCb(entity));
            rootSpatial->setLocalPosition(pos);
            rootSpatial->setLocalScale(size);
        }

        // falling block
        {
            auto modelConfig = ModelConfig::create("gltf/smallcube.glb",
                VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
                | VertexAttribute::TANGENTS
            );

            // physics
            JPH::BodyInterface& bodyInterface = physicsContext->getPhysics().GetBodyInterface();
            JPH::BoxShapeSettings shapeSettings(JPH::Vec3(.5f, .5f, .5f));
            //shapeSettings.SetEmbedded();
            JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
            auto& shape = shapeResult.Get();

            auto materialConfig = PbrMaterialConfig::create();
            materialConfig->setHasShadow(true);

            auto xCount = 4;
            auto yCount = 4;
            auto zCount = 4;
            auto yOffset = 10;
            auto zOffset = 0;
            auto padding = 0;
            for (size_t k = 0; k < yCount; k++) {
                for (size_t i = 0; i < xCount; i++) {
                    for (size_t j = 0; j < zCount; j++) {
                        glm::vec3 startingPos = glm::vec3(
                            ((1 + padding) * i) - ((1 + padding) * xCount * 0.5f),
                            ((1 + padding) * k) + yOffset,
                            ((1 + padding) * j) - ((1 + padding) * zCount * 0.5f) + zOffset
                        );

                        // physics
                        JPH::BodyCreationSettings bodySettings(shape,
                            JPH::RVec3(startingPos.x, startingPos.y, startingPos.z),
                            JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);

                        JPH::Body* body = bodyInterface.CreateBody(bodySettings);
                        bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);

                        //entity
                        auto* ecs = world->getService<entt::registry>();
                        auto entity = ecs->create();

                        auto& renderable = ecs->emplace<Component::Renderable>(entity);
                        auto& spatials = ecs->emplace<Component::Spatials>(entity);
                        ecs->emplace<Component::Rigidbody>(entity, body->GetID(), true);
                        ecs->emplace<Component::ModelComponent>(entity, modelConfig);
                        ecs->emplace<Component::Material>(entity, materialConfig);

                        auto& model = modelCache->get(modelConfig);
                        auto rootSpatial = spatials.generate(*sceneGraph, model, std::format("falling-block-{}-{}-{}", k, i, j), renderable.type);
                        rootSpatial->setChangeCb(spatialPartitioningManager->getSpatialGrid()->createCb(entity));
                        rootSpatial->setLocalPosition(startingPos);
                    }
                }
            }
        }
    }


    //// cube array
    //{
    //    auto* ecs = world->getService<entt::registry>();
    //    auto modelConfig = ModelConfig::create("gltf/smallcube.glb",
    //        VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS
    //        | VertexAttribute::TANGENTS
    //    );

    //    auto materialConfig = PbrMaterialConfig::create();
    //    materialConfig->setHasShadow(true);

    //    auto xCount = 10;
    //    auto yCount = 10;
    //    auto zCount = 10;
    //    auto yOffset = 3;
    //    auto zOffset = -5;
    //    auto sIdx = renderContext->startStaticBatch();
    //    {
    //        for (size_t k = 0; k < yCount; k++) {
    //            for (size_t i = 0; i < xCount; i++) {
    //                for (size_t j = 0; j < zCount; j++) {
    //                    auto entity = ecs->create();
    //                    auto& renderable = ecs->emplace<Component::Renderable>(entity);
    //                    renderable.setStatic();
    //                    ecs->emplace<Component::ModelComponent>(entity, modelConfig);

    //                    auto& model = modelCache->get(modelConfig);
    //                    auto& spatials = ecs->emplace<Component::Spatials>(entity);
    //                    auto rootSpatial = spatials.generate(*sceneGraph, model, "cube" + std::to_string(i), renderable.type);

    //                    //rootSpatial->setChangeCb(spatialPartitioning->getSpatialGrid()->createCb(entity));

    //                    rootSpatial->setLocalPosition(glm::vec3(
    //                        (1.5f * i) - (1.5 * xCount * 0.5f),
    //                        (1.5f * k) + yOffset,
    //                        (1.5f * j) - (1.5 * zCount * 0.5f) + zOffset
    //                    ));

    //                    spatialPartitioningManager->getSpatialGrid()->setDirty(entity);

    //                    ecs->emplace<Component::Material>(entity, materialConfig);

    //                    //renderContext->addStaticInstance(
    //                    //    model->getMeshGroups()[0]->getMesh(0),
    //                    //    *material,
    //                    //    glm::translate(glm::mat4(1.0f), glm::vec3(
    //                    //        (1.5f * i) - (1.5 * xCount * 0.5f),
    //                    //        (1.5f * k) + yOffset,
    //                    //        (1.5f * j) - (1.5 * zCount * 0.5f) + zOffset
    //                    //    )),
    //                    //    model->getMeshGroups()[0]->getMesh(0).getBounds().getSphereBounds()
    //                    //);
    //                }
    //            }
    //        }
    //    }
    //    renderContext->endStaticBatch(sIdx);
    //}

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

            pc->createPipeline<PreDrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<DrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            // terrain
            pc->createPipeline<TerrainPreDrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<TerrainDrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<TerrainDeferredOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

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

    camera->setPosition(glm::vec3(5, 7, 5));

    glm::quat camRot = camera->getRotation();
    camRot = glm::rotate(camRot, glm::radians(35.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
    auto rot = glm::rotate(camRot, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    camera->setRotation(rot);

    cameraController = std::make_unique<FreeCameraController>(inputManager);
    cameraController->setCamera(std::move(camera));
}

void TestGui::draw() {
    ImGui::Begin("Another Window");
    ImGui::Text(std::to_string(debugCtx.getIntValue("spatialGridVisibleEntities")).c_str());

    if (ImGui::Button("Toggle Debug Geometry"))
        EngineConfig::getInstance().setDebugRenderingEnabled(!EngineConfig::getInstance().isDebugRenderingEnabled());

    // physics
    {
        auto* ws = world->getSystem<PhysicsSystem>();
        if (ImGui::Button(ws->isPaused() ? "Unpause Physics" : "Pause Physics"))
            ws->setPaused(!ws->isPaused());
    }

    ImGui::End();
}
