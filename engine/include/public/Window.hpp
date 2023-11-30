#pragma once
#include <VulkanInclude.hpp>
#include <GLFW/glfw3.h>
#include <string>

class Window {

private:
    GLFWwindow* window;
    unsigned int width, height;

public:
    Window(std::string title, unsigned int width, unsigned int height);
    ~Window();

    void createSurface(VkInstance vkInstance, VkSurfaceKHR& surface);
    void pollInput();

    GLFWwindow* getWindow() {
        return window;
    }

    unsigned int getWidth() {
        return width;
    }

    unsigned int getHeight() {
        return height;
    }
};