#pragma once
#include<kengine/input/InputManager.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>

namespace EventHandler {
    using KeyDownEventHandler = std::function<bool(GLFWwindow* window, int key, int action, int mods)>;
    using KeyRepeatEventHandler = std::function<bool(GLFWwindow* window, int key, int action, int mods)>;
    using KeyUpEventHandler = std::function<bool(GLFWwindow* window, int key, int action, int mods)>;

    using ButtonDownEventHandler = std::function<bool(GLFWwindow* window, int button, int x, int y, int mods)>;
    using ButtonUpEventHandler = std::function<bool(GLFWwindow* window, int button, int x, int y, int mods)>;

    using MoveEventHandler = std::function<bool(GLFWwindow* window, int x, int y, int deltaX, int deltaY)>;
    using DragEventHandler = std::function<bool(GLFWwindow* window, int x, int y, int deltaX, int deltaY)>;
    using EnterEventHandler = std::function<bool(GLFWwindow* window, int x, int y)>;
    using ExitEventHandler = std::function<bool(GLFWwindow* window, int x, int y)>;
}

using namespace EventHandler;
class KeyEventAdapter : public KeyEventListener {
private:
    std::vector<KeyDownEventHandler> onKeyDownHandlers;
    std::vector<KeyUpEventHandler> onKeyUpHandlers;
    std::vector<KeyRepeatEventHandler> onKeyRepeatHandlers;

protected:
    InputManager& inputManager;

public:
    KeyEventAdapter(InputManager& inputManager) : inputManager(inputManager) {}

    bool onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    bool onKeyDown(GLFWwindow* window, int key, int action, int mods);
    bool onKeyUp(GLFWwindow* window, int key, int action, int mods);
    bool onKeyRepeat(GLFWwindow* window, int key, int action, int mods);

    void addKeyDownHandler(KeyDownEventHandler e) {
        onKeyDownHandlers.push_back(e);
    }

    void addKeyUpHandler(KeyUpEventHandler e) {
        onKeyUpHandlers.push_back(e);
    }

    void addKeyRepeatHandler(KeyRepeatEventHandler e) {
        onKeyRepeatHandlers.push_back(e);
    }
};

class MouseEventAdapter : public MouseEventListener {
private:
    std::vector<ButtonDownEventHandler> onButtonDownHandlers;
    std::vector<ButtonUpEventHandler> onButtonUpHandlers;
    std::vector<MoveEventHandler> onMoveHandlers;
    std::vector<DragEventHandler> onDragHandlers;

protected:
    InputManager& inputManager;

    bool dragging = false;
    int lastMousePosX = 0, lastMousePosY = 0;

public:
    MouseEventAdapter(InputManager& inputManager) : inputManager(inputManager) {}

    bool onMoveEvent(GLFWwindow* window, double x, double y) override;
    bool onButtonEvent(GLFWwindow* window, int button, int action, int mods) override;

    bool onDrag(GLFWwindow* window, int x, int y, int deltaX, int deltaY);
    bool onMove(GLFWwindow* window, int x, int y, int deltaX, int deltaY);
    bool onButtonDown(GLFWwindow* window, int button, int x, int y, int mods);
    bool onButtonUp(GLFWwindow* window, int button, int x, int y, int mods);

    void addButtonDownHandler(ButtonDownEventHandler e) {
        onButtonDownHandlers.push_back(e);
    }

    void addButtonUpHandler(ButtonUpEventHandler e) {
        onButtonUpHandlers.push_back(e);
    }

    void addMoveHandler(MoveEventHandler e) {
        onMoveHandlers.push_back(e);
    }

    void addDragHandler(DragEventHandler e) {
        onDragHandlers.push_back(e);
    }
};

class InputEventAdapter : public MouseEventListener, public KeyEventListener {
private:
    std::vector<KeyDownEventHandler> onKeyDownHandlers;
    std::vector<KeyUpEventHandler> onKeyUpHandlers;
    std::vector<KeyRepeatEventHandler> onKeyRepeatHandlers;

    std::vector<ButtonDownEventHandler> onButtonDownHandlers;
    std::vector<ButtonUpEventHandler> onButtonUpHandlers;
    std::vector<MoveEventHandler> onMoveHandlers;
    std::vector<DragEventHandler> onDragHandlers;

protected:
    InputManager& inputManager;

    bool dragging = false;
    int lastMousePosX = 0, lastMousePosY = 0;

public:
    InputEventAdapter(InputManager& inputManager) : inputManager(inputManager) {}

    bool onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    bool onMoveEvent(GLFWwindow* window, double x, double y) override;
    bool onButtonEvent(GLFWwindow* window, int button, int action, int mods) override;

    bool onKeyDown(GLFWwindow* window, int key, int action, int mods);
    bool onKeyUp(GLFWwindow* window, int key, int action, int mods);
    bool onKeyRepeat(GLFWwindow* window, int key, int action, int mods);

    bool onDrag(GLFWwindow* window, int x, int y, int deltaX, int deltaY);
    bool onMove(GLFWwindow* window, int x, int y, int deltaX, int deltaY);
    bool onButtonDown(GLFWwindow* window, int button, int x, int y, int mods);
    bool onButtonUp(GLFWwindow* window, int button, int x, int y, int mods);

    void addKeyDownHandler(KeyDownEventHandler e) {
        onKeyDownHandlers.push_back(e);
    }

    void addKeyUpHandler(KeyUpEventHandler e) {
        onKeyUpHandlers.push_back(e);
    }

    void addKeyRepeatHandler(KeyRepeatEventHandler e) {
        onKeyRepeatHandlers.push_back(e);
    }

    void addButtonDownHandler(ButtonDownEventHandler e) {
        onButtonDownHandlers.push_back(e);
    }

    void addButtonUpHandler(ButtonUpEventHandler e) {
        onButtonUpHandlers.push_back(e);
    }

    void addMoveHandler(MoveEventHandler e) {
        onMoveHandlers.push_back(e);
    }

    void addDragHandler(DragEventHandler e) {
        onDragHandlers.push_back(e);
    }
};
