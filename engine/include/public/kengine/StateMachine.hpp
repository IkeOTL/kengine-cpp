#pragma once
#include <type_traits>

template <typename E>
class State {

public:
    virtual State<E> init() = 0;
    virtual void enter(const E& parent) = 0;
    virtual void update(const E& parent) = 0;
    virtual void exit(const E& parent) = 0;

};

template <typename E>
class StateMachine {

protected:
    const E& parent;
    State<E>* globalState;
    State<E>* currentState;
    State<E>* previousState;

public:
    StateMachine(const E& parent, State<E>* globalState, State<E>* currentState, State<E>* previousState)
        : parent(parent),
        globalState(globalState),
        currentState(currentState),
        previousState(previousState) {}

    StateMachine(const E& parent)
        : parent(parent) {}

    State<E>* getCurrentState() {
        return currentState;
    }

    State<E>* getGlobalState() {
        return globalState;
    }

    State<E>* getPreviousState() {
        return previousState;
    }

    void update() {
        if (globalState)
            globalState->update(parent);

        if (currentState)
            currentState->update(parent);
    }

    void changeState(State<E>* newState) {
        previousState = currentState;

        if (currentState)
            currentState->exit(parent);

        currentState = newState;

        if (currentState)
            currentState->enter(parent);
    }

    bool revertToPreviousState() {
        if (!previousState)
            return false;

        changeState(previousState);
        return true;
    }

    void setInitialState(State<E>* state) {
        previousState = nullptr;
        currentState = state;
    }

    void setGlobalState(State<E>* state) {
        globalState = state;
    }

    bool isInState(State<E>* state) {
        return currentState == state;
    }
};