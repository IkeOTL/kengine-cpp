#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <functional>

class Window {

public:
    using WindowResizeListener = std::function<void(GLFWwindow*, int, int)>;

    Window(std::string title, unsigned int width, unsigned int height);
    ~Window();

    void createSurface(VkInstance vkInstance, VkSurfaceKHR& surface);
    void pollInput();
    void registerResizeListener(const WindowResizeListener& listener);
    void awaitEventsLoop();


    GLFWwindow* getWindow() {
        return window;
    }

    unsigned int getWidth() {
        return width;
    }

    unsigned int getHeight() {
        return height;
    }

private:
    GLFWwindow* window;
    unsigned int width, height;

    std::vector<WindowResizeListener> resizeListeners{};
};