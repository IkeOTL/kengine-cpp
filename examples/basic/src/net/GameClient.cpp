#include "GameClient.hpp"
#include <kengine/Logger.hpp>

namespace ke {
    GameClient::GameClient() {
        GameClient::callbackInstance = this;
    }

    GameClient::~GameClient() {
        // todo: should probably check connection for any active reliable messages first

        GameNetworkingSockets_Kill();
        // when we interface with steam
        // SteamDatagramClient_Kill();
    }

    void GameClient::connect() {
        // init GNS
        // when we init for steam integration its different uses `SteamDatagramClient_Init`
        {
            SteamDatagramErrMsg errMsg;
            if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
                KE_LOG_ERROR(std::format("[Net] Initialization failed: {}", errMsg));
                throw std::runtime_error(std::format("[Net] Initialization failed: {}", errMsg));
            }
        }

        socketInterface = SteamNetworkingSockets();

        SteamNetworkingIPAddr serverAddr;
        serverAddr.Clear();
        serverAddr.ParseString("127.0.0.1:8008");

// log networking output
#if KE_ACTIVE_LOG_LEVEL >= KE_LOG_LEVEL_DEBUG
        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg,
            [](ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg) {
                KE_LOG_DEBUG("[Net][" + std::to_string(eType) + "] " + pszMsg);
            });
#endif

        // connect
        {
            SteamNetworkingConfigValue_t opt;
            opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)connectionStatusChanged);

            connectionHandle = socketInterface->ConnectByIPAddress(serverAddr, 1, &opt);

            if (connectionHandle == k_HSteamNetConnection_Invalid) {
                KE_LOG_ERROR("[Net] Connection failed");
                throw std::runtime_error("[Net] Connection failed");
            }
        }
    }

    void GameClient::tick() {
        socketInterface->RunCallbacks();

        // use app state here instead of true
        while (true) {
            ISteamNetworkingMessage* recvPkts = nullptr;
            int numMsgs = socketInterface->ReceiveMessagesOnConnection(connectionHandle, &recvPkts, 10);
            if (numMsgs == 0)
                break;

            if (numMsgs < 0) {
                KE_LOG_WARN("[Net] Issue receiving messages.");
                break;
            }

            for (auto i = 0; i < numMsgs; i++) {
                auto* pkt = recvPkts + i;
                // handle message
                int x = 9;

                pkt->Release();
            }
        }
    }

    void GameClient::onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
        int i = 9;
    }
} // namespace ke