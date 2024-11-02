#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <string>
#include <unordered_map>

namespace ke {
    class GameServer {
    private:
        ISteamNetworkingSockets* socketInterface;
        HSteamListenSocket socketHandle;
        HSteamNetPollGroup pollGroupHandle;

        std::unordered_map<HSteamNetConnection, std::string> connectedClients;

        // need a better approach
        inline static GameServer* callbackInstance;
        inline static void connectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
            callbackInstance->onConnectionStatusChanged(pInfo);
        }

        void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
        void setClientName(HSteamNetConnection hConn, std::string name);

    public:
        GameServer();
        ~GameServer();

        void connect();
        void tick();
    };
} // namespace ke