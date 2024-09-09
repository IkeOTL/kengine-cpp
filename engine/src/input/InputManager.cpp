#include <kengine/input/InputManager.hpp>

void InputManager::onMoveEvent(GLFWwindow* window, double xpos, double ypos) {
    mouseX = (int)xpos;
    mouseY = (int)ypos;

    if (activeMouseListener) {
        activeMouseListener->onMoveEvent(window, xpos, ypos);
        return;
    }

    for (auto* l : mouseEventListeners)
        l->onMoveEvent(window, xpos, ypos);
}

void InputManager::onButtonEvent(GLFWwindow* window, int button, int action, int mods) {
    mouseButtonDown[button] = action == GLFW_PRESS;

    if (activeMouseListener) {
        activeMouseListener->onButtonEvent(window, button, action, mods);
        return;
    }

    for (auto* l : mouseEventListeners)
        l->onButtonEvent(window, button, action, mods);
}

void InputManager::onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == -1)
        return;

    keyDown[key] = action == GLFW_PRESS || action == GLFW_REPEAT;

    if (activeKeyListener) {
        activeKeyListener->onKeyEvent(window, key, scancode, action, mods);
        return;
    }

    for (auto* l : keyEventListeners)
        l->onKeyEvent(window, key, scancode, action, mods);
}

void InputManager::onCharEvent(GLFWwindow* window, int codepoint) {
    if (!activeCharEventListener)
        return;

    activeCharEventListener->onCharEvent(window, codepoint);
}