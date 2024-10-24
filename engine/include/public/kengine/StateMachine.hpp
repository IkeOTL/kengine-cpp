#pragma once
#include <type_traits>

namespace ke {
    template <typename E>
    class State {
    public:
        virtual ~State() = default;

        virtual void init() = 0;
        virtual void enter(E& parent) = 0;
        virtual void update(E& parent) = 0;
        virtual void exit(E& parent) = 0;
    };

    template <typename E>
    class StateMachine {

    private:
        E& parent;
        State<E>* globalState;
        State<E>* currentState;
        State<E>* previousState;

    public:
        StateMachine(E& parent, State<E>* globalState, State<E>* currentState, State<E>* previousState)
            : parent(parent),
            globalState(globalState),
            currentState(currentState),
            previousState(previousState) {}

        StateMachine(E& parent)
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
} // namespace ke