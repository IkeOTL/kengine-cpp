#pragma once
#include <kengine/ecs/BaseSystem.hpp>
#include <kengine/ecs/World.hpp>
#include <thirdparty/entt.hpp>

class EcsSystem : public BaseSystem {
private:
    entt::registry* ecs;

protected:
    virtual void begin() override {}
    virtual void end() override {}

    entt::registry& getEcs() {
        return *ecs;
    }

public:
    virtual void init() = 0;
    virtual void processSystem(float delta) override = 0;

    void initialize() override {
        ecs = getWorld().getService<entt::registry>();

        if (!ecs)
            throw std::runtime_error("World does not have a `entt::registry` registered service.");

        init();
    }
};