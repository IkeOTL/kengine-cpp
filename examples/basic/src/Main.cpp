#include "BasicGameTest.hpp"

#include <glm/glm.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/math.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/renderpass/CascadeShadowMapRenderPass.hpp>
#include <steam/steamnetworkingsockets.h>
#include <kengine/Logger.hpp>
#include <kengine/EngineConfig.hpp>

int main() {
    KE_LOG_INFO("Example started.");

    //EngineConfig::getInstance().setDebugRenderingEnabled(true);
    EngineConfig::getInstance().setAssetRoot("../res/");

    BasicGameTest game;
    game.run();

    return 0;
}