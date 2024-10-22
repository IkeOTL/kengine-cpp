#pragma once

class World;

class BaseSystem {
private:
    friend class World;
    World* world = nullptr;

protected:
    World& getWorld() const {
        return *world;
    }

    virtual bool checkProcessing() {
        return true;
    }

public:
    BaseSystem() = default;

    // called by the world
    virtual void initialize() = 0;

    /// <summary>
    /// No checks, just starts processing
    /// </summary>
    virtual void processSystem(float delta) = 0;

    /// <summary>
    /// First checks if system should be processed
    /// </summary>
    void process(float delta) {
        if (!checkProcessing())
            return;

        processSystem(delta);
    }
};