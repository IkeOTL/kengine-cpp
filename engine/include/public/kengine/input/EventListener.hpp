#pragma once
#include <GLFW/glfw3.h>
#include <vector>

namespace ke {
    class MouseEventListener {
    public:
        virtual bool onMoveEvent(GLFWwindow* window, double xpos, double ypos) = 0;
        virtual bool onButtonEvent(GLFWwindow* window, int button, int action, int mods) = 0;
    };

    class KeyEventListener {
    public:
        virtual bool onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
    };

    class CharacterEventListener {
    public:
        virtual bool onCharEvent(GLFWwindow* window, unsigned int codepoint) = 0;
    };
} // namespace ke


