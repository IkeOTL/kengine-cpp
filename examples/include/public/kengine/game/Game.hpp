#pragma once
#include <kengine/StateMachine.hpp>

struct SceneTime {
private:
    float sceneTime = 0;
    float delta = 0;
    float alpha = 0;

public:
    void addSceneTime(float t) {
        sceneTime += t;
    }

    float getSceneTime() const {
        return sceneTime;
    }

    float getDelta() const {
        return delta;
    }

    void setDelta(float delta) {
        this->delta = delta;
    }

    float getAlpha() const {
        return alpha;
    }

    void setAlpha(float delta) {
        this->alpha = alpha;
    }
};

class Game {
public:
    virtual float getDelta() = 0;
    virtual void run() = 0;
};