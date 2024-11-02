#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <memory>

namespace ke {
    class GameClient {
    private:
        ISteamNetworkingSockets* socketInterface;
        HSteamNetConnection connectionHandle;

        // need a better approach
        inline static GameClient* callbackInstance;
        inline static void connectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
            callbackInstance->onConnectionStatusChanged(pInfo);
        }

        void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

    public:
        GameClient();
        ~GameClient();

        void connect();
        void tick();
    };
} // namespace ke