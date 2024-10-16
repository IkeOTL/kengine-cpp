#pragma once
#include <thirdparty/entt.hpp>
#include <memory>


class MyPlayerContext {
private:
    entt::entity playerEntity = entt::null;

public:
    inline static std::unique_ptr<MyPlayerContext> create() {
        return std::make_unique<MyPlayerContext>();
    }

    entt::entity getPlayerEntityId() {
        return playerEntity;
    }

    void setPlayerEntityId(entt::entity e) {
        playerEntity = e;
    }
};