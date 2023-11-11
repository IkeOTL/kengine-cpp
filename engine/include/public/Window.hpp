#pragma once
#include <VulkanInclude.hpp>
#include <GLFW/glfw3.h>

class Window {

private:
    GLFWwindow* window;

public:
    Window();
    ~Window();

    void createSurface(VkInstance vkInstance, VkSurfaceKHR& surface);
    void pollInput();

    GLFWwindow* getWindow() {
        return window;
    }
};