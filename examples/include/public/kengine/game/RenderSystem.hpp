#pragma once
#include <kengine/ecs/BaseSystem.hpp>

class World;

class RenderSystem : public BaseSystem {
private:


public:
    RenderSystem() = default;

    void init() override;
    void processSystem() override;
};