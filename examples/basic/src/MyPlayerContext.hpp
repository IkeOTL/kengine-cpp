#pragma once
#include "PhysicsContext.hpp"
#include <jolt/Physics/Character/CharacterVirtual.h>
#include <thirdparty/entt.hpp>
#include <memory>


class MyPlayerContext {
private:
    entt::entity playerEntity = entt::null;

    // physics
    PhysicsContext& physicsContext;
    std::unique_ptr<JPH::CharacterVirtual> physicsCharacter;
    std::unique_ptr<JPH::CharacterVirtualSettings> physicsCharacterSettings;

public:
    MyPlayerContext(PhysicsContext& physicsContext)
        : physicsContext(physicsContext) {}

    static std::unique_ptr<MyPlayerContext> create(PhysicsContext& physicsContext);

    entt::entity getPlayerEntityId() {
        return playerEntity;
    }

    JPH::CharacterVirtual& getPlayerPhysicsBody() {
        return *physicsCharacter;
    }

    void setPlayerEntityId(entt::entity e) {
        playerEntity = e;
    }
};