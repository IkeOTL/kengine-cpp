#include "GameServer.hpp"
#include <kengine/Logger.hpp>

namespace ke {
    GameServer::GameServer() {
    }

    GameServer::~GameServer() {
    }

    void GameServer::connect() {
        // init GNS
        // when we init for steam integration its different uses `SteamDatagramClient_Init`
        {
            SteamDatagramErrMsg errMsg;
            if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
                KE_LOG_ERROR(std::format("[Net] Initialization failed: {}", errMsg));
                throw std::runtime_error(std::format("[Net] Initialization failed: {}", errMsg));
            }
        }

        auto port = 8008;
        socketInterface = SteamNetworkingSockets();

        SteamNetworkingIPAddr serverLocalAddr;
        serverLocalAddr.Clear();
        serverLocalAddr.m_port = port;

        // create "socket" (not really a socket since this uses UDP)
        {
            SteamNetworkingConfigValue_t opt;
            opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)connectionStatusChanged);
            socketHandle = socketInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
            if (socketHandle == k_HSteamListenSocket_Invalid) {
                KE_LOG_ERROR(std::format("[Net] Failed to listen on port {}", port));
                throw std::runtime_error(std::format("[Net] Failed to listen on port {}", port));
            }
        }

        pollGroupHandle = socketInterface->CreatePollGroup();
    }

    void GameServer::tick() {
        socketInterface->RunCallbacks();

        while (true) {
            ISteamNetworkingMessage* recvPkts = nullptr;
            int numMsgs = socketInterface->ReceiveMessagesOnPollGroup(pollGroupHandle, &recvPkts, 10);
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

    void GameServer::onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
        int i = 9;
    }

    void GameServer::setClientName(HSteamNetConnection connHandle, std::string name) {
        socketInterface->SetConnectionName(connHandle, name.c_str());
    }

} // namespace ke