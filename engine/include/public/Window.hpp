#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

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