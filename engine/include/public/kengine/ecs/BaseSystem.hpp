#pragma once

class EcsWorld;

class BaseSystem {
private:
    EcsWorld& ecsWorld;

protected:

    virtual void processSystem() = 0;

    virtual void begin() {}
    virtual void end() {}

    virtual bool checkProcessing() {
        return true;
    }

public:
    BaseSystem(EcsWorld& ecsWorld) : ecsWorld(ecsWorld) {}

    // called by the world
    virtual void init() = 0;

    void process() {
        if (!checkProcessing())
            return;

        begin();
        processSystem();
        end();
    }
};