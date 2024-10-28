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
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.cpp>
#include <components/Physics.hpp>
#include <PhysicsSyncSystem.hpp>
#include <PlayerCameraSystem.hpp>


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
            threadPool.reset();
            vkDeviceWaitIdle(vulkanCxt->getVkDevice());
            break;
        }

        auto newTime = high_resolution_clock::now();
        delta = duration_cast<nanoseconds>(newTime - lastFrame).count() * .000000001f;
        lastFrame = newTime;

        // cap delta
        delta = ke::math::min(delta, .2f);

        sm.update();
    }
    //    });

    //renderThread.join();
}

std::unique_ptr<ke::State<ke::Game>> BasicGameTest::init() {
    window = ke::Window::create("rawr", 1920, 1080);
    inputManager = ke::InputManager::create();
    window->setInputManager(inputManager.get());

    initVulkan(*window);

    // review this usage
    threadPool.reset(new ke::ExecutorService(4, [&]() {
        vulkanCxt->getCommandPool()->initThread(*vulkanCxt);
        }));

    debugContext = ke::DebugContext::create();
    initCamera(*inputManager, *debugContext);

    vulkanCxt->setDebugContext(debugContext.get());

    djm = ke::DeferredJobManager::create();

    assetIo = ke::FileSystemAssetIO::create();
    lightsManager = ke::LightsManager::create(*cameraController);
    sceneTime = std::make_unique<ke::SceneTime>();
    sceneGraph = ke::SceneGraph::create();
    spatialPartitioningManager = ke::SpatialPartitioningManager::create();
    skeletonManager = ke::SkeletonManager::create(*vulkanCxt);

    eventBus = ke::EventBus::create(*sceneTime);
    //eventBus->registerHandler([](const auto& evt, auto& w) {});

    spatialPartitioningManager->setSpatialGrid(ke::SpatialGrid::create(64, 64, 16));

    modelFactory = ke::GltfModelFactory::create(*vulkanCxt, *assetIo);

    modelCache = ke::AsyncModelCache::create(*modelFactory, *threadPool);
    animationFactory = ke::GltfAnimationFactory::create(*vulkanCxt, *assetIo);
    animationCache = ke::AsyncAnimationCache::create(*animationFactory, *threadPool);
    textureFactory = ke::TextureFactory::create(*vulkanCxt, *assetIo);
    textureCache = ke::AsyncTextureCache::create(*textureFactory, *threadPool);
    materialCache = ke::AsyncMaterialCache::create(vulkanCxt->getPipelineCache(), *textureCache, vulkanCxt->getGpuBufferCache(), *threadPool);


    imGuiContext = std::make_unique<TestGui>(*vulkanCxt, *sceneTime, *debugContext);
    imGuiContext->init(*window);

    terrainContext = std::make_unique<ke::TerrainContext>(*materialCache);

    renderContext = ke::RenderContext::create(*vulkanCxt, *lightsManager, *cameraController);
    renderContext->init(terrainContext.get());
    renderContext->setImGuiContext(imGuiContext.get());

    physicsContext = PhysicsContext::create();
    physicsContext->init();

    myPlayerContext = MyPlayerContext::create(*physicsContext);
    playerMovementManager = PlayerMovementManager::create();

    world = ke::World::create(ke::WorldConfig()
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
        .addService(cameraController.get())
        .addService(terrainContext.get())

        .addService(threadPool.get())
        .addService(assetIo.get())
        .addService(lightsManager.get())
        .addService(skeletonManager.get())

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
        .setSystem<PlayerCameraSystem>()
        .setSystem<RenderSystem>()
    );

    // pause physics by defaut
    physicsContext->setPaused(true);

    // player dummy
    {
        auto* ecs = world->getService<entt::registry>();
        auto modelConfig = ke::ModelConfig::create("gltf/smallcube.glb",
            ke::VertexAttribute::POSITION | ke::VertexAttribute::NORMAL | ke::VertexAttribute::TEX_COORDS
            | ke::VertexAttribute::TANGENTS
        );

        auto materialConfig = ke::PbrMaterialConfig::create();
        materialConfig->setHasShadow(true);

        auto entity = ecs->create();
        myPlayerContext->setPlayerEntityId(entity);

        auto& renderable = ecs->emplace<Component::Renderable>(entity);
        auto& spatials = ecs->emplace<Component::Spatials>(entity);
        ecs->emplace<Component::ModelComponent>(entity, modelConfig);
        ecs->emplace<Component::Material>(entity, materialConfig);
        ecs->emplace<Component::LinearVelocity>(entity);
        ecs->emplace<Component::TerrainGrounded>(entity);

        auto& model = modelCache->get(modelConfig);
        auto playerSpatial = sceneGraph->create("player");
        auto modelSpatial = spatials.generate(*sceneGraph, model, "playerMesh", renderable.type);
        playerSpatial->addChild(modelSpatial);
        spatials.rootSpatialId = playerSpatial->getSceneId();
        modelSpatial->setChangeCb(spatialPartitioningManager->getSpatialGrid()->createCb(entity));
        modelSpatial->setLocalPosition(glm::vec3(0, 1, 0));
        modelSpatial->setLocalScale(glm::vec3(0.5f, 2.0f, 0.5f));
    }

    // physics experiment
    {
        // terrain heightfield physics shape
        {
            auto& terrain = terrainContext->getTerrain();
            auto terrainWidth = terrain.getTerrainHeightsWidth();
            auto terrainLength = terrain.getTerrainHeightsLength();
            auto& terrainHeights = terrain.getHeights();
            auto terrainUnitSize = terrain.getUnitSize();

            std::vector<float> pHeights;
            pHeights.reserve(terrainWidth * terrainLength);
            // Convert compressed heights to floating point
            for (size_t i = 0; i < terrainHeights.size(); ++i) {
                float scaledHeight = terrainHeights[i] / terrainUnitSize;
                pHeights.push_back(scaledHeight);
            }

            auto cellSize = 1.0f;
            auto xStep = 1.0f;
            auto zStep = 1.0f;
            JPH::Vec3 pTerrainOffset(terrain.getWorldOffsetX(), 0, terrain.getWorldOffsetZ());
            JPH::Vec3 pTerrainScale(cellSize, 1, cellSize);
            JPH::uint pTerrainSize = terrainWidth; // Jolt only supports square terrain
            JPH::PhysicsMaterialList pMaterials;

            JPH::HeightFieldShapeSettings settings(
                pHeights.data(), pTerrainOffset, pTerrainScale, pTerrainSize, nullptr, pMaterials);

            settings.mBlockSize = 8;
            settings.mBitsPerSample = 8;

            JPH::BodyInterface& bodyInterface = physicsContext->getPhysics().GetBodyInterface();
            auto mHeightField = StaticCast<JPH::HeightFieldShape>(settings.Create().Get());
            bodyInterface.CreateAndAddBody(JPH::BodyCreationSettings(mHeightField, JPH::RVec3::sZero(), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::TERRAIN), JPH::EActivation::DontActivate);
        }

        // falling block
        {
            auto modelConfig = ke::ModelConfig::create("gltf/smallcube.glb",
                ke::VertexAttribute::POSITION | ke::VertexAttribute::NORMAL | ke::VertexAttribute::TEX_COORDS
                | ke::VertexAttribute::TANGENTS
            );

            // physics
            JPH::BodyInterface& bodyInterface = physicsContext->getPhysics().GetBodyInterface();
            JPH::BoxShapeSettings shapeSettings(JPH::Vec3(.5f, .5f, .5f));
            //shapeSettings.SetEmbedded();
            JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
            auto& shape = shapeResult.Get();

            JPH::MassProperties msp;
            msp.ScaleToMass(10.0f); //actual mass in kg


            auto materialConfig = ke::PbrMaterialConfig::create();
            materialConfig->setHasShadow(true);

            auto xCount = 5;
            auto yCount = 3;
            auto zCount = 3;
            auto yOffset = 20;
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

                        bodySettings.mMassPropertiesOverride = msp;
                        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;

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

    return std::make_unique<MainGameState>(*world);
}

void BasicGameTest::initVulkan(ke::Window& window) {
    vulkanCxt = ke::VulkanContext::create(
        window,
        [](VkDevice vkDevice, ke::ColorFormatAndSpace& cfs) {
            std::vector<std::unique_ptr<ke::RenderPass>> passes;
            passes.emplace_back(ke::DeferredPbrRenderPass::create(vkDevice, cfs));
            passes.emplace_back(ke::CascadeShadowMapRenderPass::create(vkDevice, cfs));
            return passes;
        },
        [](ke::VulkanContext& vkCtx, std::vector<std::unique_ptr<ke::RenderPass>>& rp) {
            auto pc = ke::PipelineCache::create(vkCtx.getVkDevice());

            pc->createPipeline<ke::DeferredOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<ke::SkinnedOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<ke::DeferredCompositionPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<ke::CascadeShadowMapPipeline>()
                .init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });

            pc->createPipeline<ke::SkinnedCascadeShadowMapPipeline>()
                .init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });

            pc->createPipeline<ke::PreDrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<ke::DrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            // terrain
            pc->createPipeline<ke::TerrainPreDrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<ke::TerrainDrawCullingPipeline>()
                .init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});

            pc->createPipeline<ke::TerrainDeferredOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});

#ifdef KE_DEBUG_RENDER
            pc->createPipeline<ke::DebugDeferredOffscreenPbrPipeline>()
                .init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
#endif

            return pc;
        },
        [](ke::VulkanContext& vkCxt, ke::Swapchain& swapchain, std::vector<std::unique_ptr<ke::RenderPass>>& renderPasses) {
            auto& rp = renderPasses[0];
            auto sc = vkCxt.getSwapchain();
            rp->createRenderTargets(vkCxt.getVmaAllocator(), sc->getImageViews(), sc->getExtents());
        }
    );

    vulkanCxt->init(true);
}

void BasicGameTest::initCamera(ke::InputManager& inputManager, ke::DebugContext& dbg) {
    auto fov = glm::radians(60.0f);
    auto aspectRatio = (float)window->getWidth() / window->getHeight();
    auto camera = ke::Camera::create(fov, aspectRatio, ke::Camera::NEAR_CLIP, ke::Camera::FAR_CLIP);

    camera->setPosition(glm::vec3(5, 7, 5));

    glm::quat camRot = camera->getRotation();
    camRot = glm::rotate(camRot, glm::radians(35.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
    auto rot = glm::rotate(camRot, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    camera->setRotation(rot);

    //cameraController = std::make_unique<FreeCameraController>(inputManager);
    cameraController = std::make_unique<PlayerCameraController>();
    cameraController->setCamera(std::move(camera));
}

void TestGui::draw() {
    ImGui::Begin("Another Window");
    ImGui::Text(std::to_string(debugCtx.getIntValue("spatialGridVisibleEntities")).c_str());

    if (ImGui::Button("Toggle Debug Geometry"))
        ke::EngineConfig::getInstance().setDebugRenderingEnabled(!ke::EngineConfig::getInstance().isDebugRenderingEnabled());

    // physics
    {
        auto* ws = world->getService<PhysicsContext>();
        if (ImGui::Button(ws->isPaused() ? "Unpause Physics" : "Pause Physics"))
            ws->setPaused(!ws->isPaused());
    }

    ImGui::End();
}
