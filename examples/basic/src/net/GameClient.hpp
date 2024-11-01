#pragma once

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <memory>


namespace ke {
    class GameClient {
    private:
        std::unique_ptr<ISteamNetworkingSockets> socketInterface;
        HSteamNetConnection connectionHandle;

    public:
        void init();
    };
} // namespace ke