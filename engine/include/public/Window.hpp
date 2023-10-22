#pragma once
#include <GLFW/glfw3.h>

class Window {

private:
    GLFWwindow* window;

public:
    Window();
    ~Window();
    void pollInput();

    GLFWwindow* getWindow() {
        return window;
    }
};