#pragma once
#include <type_traits>

template <typename E>
class State {

public:
    virtual State init() = 0;
    virtual void enter(E parent) = 0;
    virtual void update(E parent) = 0;
    virtual void exit(E parent) = 0;

};

template <typename E, typename S>
class StateMachine {
    static_assert(std::is_base_of<State<E>, S>::value, "S must be a subclass of State<E>");

public:
    virtual void update() = 0;
    virtual void changeState(S newState) = 0;
    virtual bool revertToPreviousState() = 0;
    virtual void setInitialState(S state) = 0;
    virtual void setGlobalState(S state) = 0;
    virtual S getCurrentState() = 0;
    virtual S getGlobalState() = 0;
    virtual S getPreviousState() = 0;
    virtual bool isInState(S state) = 0;

};

template <typename E, typename S>
class DefaultStateMachine : public StateMachine<E, S> {
    static_assert(std::is_base_of<State<E>, S>::value, "S must be a subclass of State<E>");

protected:
    should be pounters?
    E parent;
    S globalState;
    S currentState;
    S previousState;

public:
    DefaultStateMachine(DefaultStateMachine(E parent, S globalState, S currentState, S previousState))
        : parent(parent),
        globalState(globalState),
        currentState(currentState),
        previousState()
    {}

    void update() override;
    void changeState(S newState) override;
    bool revertToPreviousState() override;
    void setInitialState(S state) override;
    void setGlobalState(S state) override;
    S getCurrentState() override;
    S getGlobalState() override;
    S getPreviousState() override;
    bool isInState(S state) override;

};