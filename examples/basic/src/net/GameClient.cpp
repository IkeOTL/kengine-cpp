#include "GameClient.hpp"
#include <kengine/Logger.hpp>

static void logOutput(ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg) {
    KE_LOG_DEBUG(std::format("[Net][{}] {}", eType, pszMsg));
}

namespace ke {
    GameClient::~GameClient() {
        // todo: should probably check connection for any active reliable messages first

        GameNetworkingSockets_Kill();
        // when we interface with steam
        // SteamDatagramClient_Kill();
    }

    void GameClient::init() {
        SteamNetworkingIPAddr serverAddr;
        serverAddr.Clear();
        serverAddr.ParseString("127.0.0.1:8008");

        // init GNS
        // when we init for steam integration its different uses `SteamDatagramClient_Init`
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
            KE_LOG_ERROR(std::format("[Net] Initialization failed: {}", errMsg));
            throw std::runtime_error(std::format("[Net] Initialization failed: {}", errMsg));
        }

        // log networking output
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg,
            [](ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg) {
                KE_LOG_DEBUG(std::format("[Net][{}] {}", eType, pszMsg));
            });

        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)connectionStatusChangedCallback);

        connectionHandle = socketInterface->ConnectByIPAddress(serverAddr, 1, &opt);
        if (connectionHandle == k_HSteamNetConnection_Invalid) {
            KE_LOG_ERROR("[Net] Connection failed");
            throw std::runtime_error("[Net] Connection failed");
        }
    }
} // namespace ke