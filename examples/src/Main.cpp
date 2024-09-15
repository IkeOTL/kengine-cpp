
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
#include <kengine/game/BasicGameTest.hpp>
#include <steam/steamnetworkingsockets.h>
#include <kengine/Logger.hpp>

static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    KE_LOG_INFO("somethign happen??");
}

int main() {
    KE_LOG_INFO("Example started.");

    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
        KE_LOG_ERROR(errMsg);

    auto m_pInterface = SteamNetworkingSockets();

    SteamNetworkingIPAddr serverLocalAddr;
    serverLocalAddr.Clear();
    serverLocalAddr.m_port = 8008;
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
    auto hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
    if (hListenSock == k_HSteamListenSocket_Invalid)
        KE_LOG_ERROR("k_HSteamListenSocket_Invalid");


    BasicGameTest game;
    game.run();

    return 0;
}