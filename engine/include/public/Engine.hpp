#pragma once
#include <GLFW/glfw3.h>

class Engine {
public:
    Engine();
    void run();
    ~Engine();

private:
    GLFWwindow* window;
};