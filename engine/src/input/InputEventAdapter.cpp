#include <kengine/input/InputEventAdapter.hpp>

namespace ke {
    // KeyEventAdapter
    bool KeyEventAdapter::onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
        switch (action) {
        case GLFW_PRESS:
            return onKeyDown(window, key, action, mods);
        case GLFW_RELEASE:
            return onKeyUp(window, key, action, mods);
        case GLFW_REPEAT:
            return onKeyRepeat(window, key, action, mods);
        default:
            return false;
        };
    }

    bool KeyEventAdapter::onKeyDown(GLFWwindow* window, int key, int action, int mods) {
        for (auto& e : this->onKeyDownHandlers)
            if (e(window, key, action, mods))
                return true;

        return false;
    }

    bool KeyEventAdapter::onKeyUp(GLFWwindow* window, int key, int action, int mods) {
        for (auto& e : this->onKeyUpHandlers)
            if (e(window, key, action, mods))
                return true;

        return false;
    }

    bool KeyEventAdapter::onKeyRepeat(GLFWwindow* window, int key, int action, int mods) {
        for (auto& e : this->onKeyRepeatHandlers)
            if (e(window, key, action, mods))
                return true;

        return false;
    }

    // MouseEventAdapter
    bool MouseEventAdapter::onMoveEvent(GLFWwindow* window, double x, double y) {
        auto deltaX = (int)(x - lastMousePosX);
        auto deltaY = (int)(y - lastMousePosY);

        lastMousePosX = (int)x;
        lastMousePosY = (int)y;

        if (dragging) {
            return onDrag(window, lastMousePosX, lastMousePosY, deltaX, deltaY);
        }

        return onMove(window, lastMousePosX, lastMousePosY, deltaX, deltaY);
    }

    bool MouseEventAdapter::onButtonEvent(GLFWwindow* window, int button, int action, int mods) {
        dragging = action == GLFW_PRESS;

        inputManager.setActiveMouseListener(dragging ? this : nullptr);

        if (dragging)
            return onButtonDown(window, button, lastMousePosX, lastMousePosY, mods);

        return onButtonUp(window, button, lastMousePosX, lastMousePosY, mods);
    }

    bool MouseEventAdapter::onDrag(GLFWwindow* window, int x, int y, int deltaX, int deltaY) {
        for (auto& e : onDragHandlers)
            if (e(window, x, y, deltaX, deltaY))
                return true;

        return false;
    }

    bool MouseEventAdapter::onMove(GLFWwindow* window, int x, int y, int deltaX, int deltaY) {
        for (auto& e : onMoveHandlers)
            if (e(window, x, y, deltaX, deltaY))
                return true;

        return false;
    }

    bool MouseEventAdapter::onButtonDown(GLFWwindow* window, int button, int x, int y, int mods) {
        for (auto& e : onButtonDownHandlers)
            if (e(window, button, x, y, mods))
                return true;

        return false;
    }

    bool MouseEventAdapter::onButtonUp(GLFWwindow* window, int button, int x, int y, int mods) {
        for (auto& e : onButtonUpHandlers)
            if (e(window, button, x, y, mods))
                return true;

        return false;
    }

    // InputEventAdapter
    bool InputEventAdapter::onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
        switch (action) {
        case GLFW_PRESS:
            return onKeyDown(window, key, action, mods);
        case GLFW_RELEASE:
            return onKeyUp(window, key, action, mods);
        case GLFW_REPEAT:
            return onKeyRepeat(window, key, action, mods);
        default:
            return false;
        };
    }

    bool InputEventAdapter::onMoveEvent(GLFWwindow* window, double x, double y) {
        auto deltaX = (int)(x - lastMousePosX);
        auto deltaY = (int)(y - lastMousePosY);

        lastMousePosX = (int)x;
        lastMousePosY = (int)y;

        if (dragging) {
            return onDrag(window, lastMousePosX, lastMousePosY, deltaX, deltaY);
        }

        return onMove(window, lastMousePosX, lastMousePosY, deltaX, deltaY);
    }

    bool InputEventAdapter::onButtonEvent(GLFWwindow* window, int button, int action, int mods) {
        dragging = action == GLFW_PRESS;

        inputManager.setActiveMouseListener(dragging ? this : nullptr);

        if (dragging)
            return onButtonDown(window, button, lastMousePosX, lastMousePosY, mods);

        return onButtonUp(window, button, lastMousePosX, lastMousePosY, mods);
    }

    bool InputEventAdapter::onKeyDown(GLFWwindow* window, int key, int action, int mods) {
        for (auto& e : this->onKeyDownHandlers)
            if (e(window, key, action, mods))
                return true;

        return false;
    }

    bool InputEventAdapter::onKeyUp(GLFWwindow* window, int key, int action, int mods) {
        for (auto& e : this->onKeyUpHandlers)
            if (e(window, key, action, mods))
                return true;

        return false;
    }

    bool InputEventAdapter::onKeyRepeat(GLFWwindow* window, int key, int action, int mods) {
        for (auto& e : this->onKeyRepeatHandlers)
            if (e(window, key, action, mods))
                return true;

        return false;
    }

    bool InputEventAdapter::onDrag(GLFWwindow* window, int x, int y, int deltaX, int deltaY) {
        for (auto& e : onDragHandlers)
            if (e(window, x, y, deltaX, deltaY))
                return true;

        return false;
    }

    bool InputEventAdapter::onMove(GLFWwindow* window, int x, int y, int deltaX, int deltaY) {
        for (auto& e : onMoveHandlers)
            if (e(window, x, y, deltaX, deltaY))
                return true;

        return false;
    }

    bool InputEventAdapter::onButtonDown(GLFWwindow* window, int button, int x, int y, int mods) {
        for (auto& e : onButtonDownHandlers)
            if (e(window, button, x, y, mods))
                return true;

        return false;
    }

    bool InputEventAdapter::onButtonUp(GLFWwindow* window, int button, int x, int y, int mods) {
        for (auto& e : onButtonUpHandlers)
            if (e(window, button, x, y, mods))
                return true;

        return false;
    }
} // namespace ke