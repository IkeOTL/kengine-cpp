#pragma once
#include <kengine/StateMachine.hpp>

class Game {
public:
    virtual float getDelta() = 0;
    virtual void run() = 0;
};