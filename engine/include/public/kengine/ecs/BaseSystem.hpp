#pragma once

class World;

class BaseSystem {
private:
    friend class World;
    World* world;

protected:
    virtual void begin() {}
    virtual void end() {}

    virtual bool checkProcessing() {
        return true;
    }

public:
    BaseSystem() = default;

    // called by the world
    virtual void init() = 0;
    virtual void processSystem() = 0;

    void process() {
        if (!checkProcessing())
            return;

        begin();
        processSystem();
        end();
    }
};