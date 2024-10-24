#pragma once
#include <kengine/ecs/EcsSystem.hpp>
#include <MyPlayerContext.hpp>


class MyPlayerSystem : public ke::EcsSystem {
private:
    MyPlayerContext* playerCtx;

public:
    virtual void init() override = 0;

    void initialize() override {
        playerCtx = getService<MyPlayerContext>();

        assert(playerCtx);

        ke::EcsSystem::initialize();
    }

    virtual void processSystem(entt::entity playerEntity) = 0;

    void processSystem(float delta) override {
        auto eId = playerCtx->getPlayerEntityId();

        if (eId == entt::null)
            return;

        processSystem(eId);
    }
};