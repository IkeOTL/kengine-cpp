#pragma once
#include <thirdparty/entt.hpp>

class BaseSystem {
private:
    entt::registry& registry;

protected:
    virtual void init() = 0;

    virtual void processSystem() = 0;

    virtual void begin() {}
    virtual void end() {}

    virtual bool checkProcessing() {
        return true;
    }

public:
    BaseSystem(entt::registry& registry) : registry(registry) {}

    void process() {
        if (!checkProcessing())
            return;

        begin();
        processSystem();
        end();
    }
};