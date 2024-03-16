#pragma once
#include <kengine/input/EventListener.hpp>
#include <GLFW/glfw3.h>
#include <vector>

class InputManager {
private:
    std::vector<MouseEventListener*> mouseEventListeners;
    std::vector<KeyEventListener*> keyEventListeners;

    int mouseX = 0, mouseY = 0;
    bool keyDown[GLFW_KEY_LAST + 1];
    bool mouseButtonDown[GLFW_MOUSE_BUTTON_LAST + 1];

    KeyEventListener* activeKeyListener = nullptr;
    MouseEventListener* activeMouseListener = nullptr;
    CharacterEventListener* activeCharEventListener = nullptr;

public:
    void onMoveEvent(GLFWwindow* window, double xpos, double ypos);
    void onButtonEvent(GLFWwindow* window, int button, int action, int mods);
    void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
    void onCharEvent(GLFWwindow* window, int codepoint);

    bool isKeyDown(int key) const {
        return keyDown[key];
    }

    bool isMouseButtonDown(int key) const {
        return mouseButtonDown[key];
    }

    int getMouseX() const {
        return mouseX;
    }

    int getMouseY() const {
        return mouseY;
    }

    MouseEventListener* getActiveMouseListener() {
        return activeMouseListener;
    }

    void setActiveMouseListener(MouseEventListener* l) {
        activeMouseListener = l;
    }

    void setActiveKeyListener(KeyEventListener* l) {
        activeKeyListener = l;
    }

    CharacterEventListener* getActiveCharEventListener() {
        return activeCharEventListener;
    }

    void setActiveCharEventListener(CharacterEventListener* l) {
        activeCharEventListener = l;
    }

    void registerKeyEventListener(KeyEventListener* listener) {
        keyEventListeners.push_back(listener);
    }

    void registerMouseEventListener(MouseEventListener* listener) {
        mouseEventListeners.push_back(listener);
    }

    void unregisterMouseEventListener(MouseEventListener* listener) {
        mouseEventListeners.push_back(listener);
    }
};