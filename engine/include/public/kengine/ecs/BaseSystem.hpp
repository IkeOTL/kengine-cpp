#pragma once
#include <thirdparty/entt.hpp>

class BaseSystem {
private:
    entt::registry& registry;

public:
    BaseSystem(entt::registry& registry) : registry(registry) {}

protected:

    virtual void processSystem() = 0;

    virtual void begin() {}
    virtual void end() {}

    virtual bool checkProcessing() {
        return true;
    }
};