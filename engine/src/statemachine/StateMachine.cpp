#include <kengine/statemachine/StateMachine.hpp>
#include <iostream>

template <typename E, typename S>
void DefaultStateMachine<E, S>::update()  {

}

template <typename E, typename S>
void DefaultStateMachine<E, S>::changeState(S newState) {

}

template <typename E, typename S>
bool DefaultStateMachine<E, S>::revertToPreviousState() {

}

template <typename E, typename S>
void DefaultStateMachine<E, S>::setInitialState(S state) {

}

template <typename E, typename S>
void DefaultStateMachine<E, S>::setGlobalState(S state) {

}

template <typename E, typename S>
S DefaultStateMachine<E, S>::getCurrentState() {

}

template <typename E, typename S>
S DefaultStateMachine<E, S>::getGlobalState() {

}

template <typename E, typename S>
S DefaultStateMachine<E, S>::getPreviousState() {

}

template <typename E, typename S>
bool DefaultStateMachine<E, S>::isInState(S state) {

}