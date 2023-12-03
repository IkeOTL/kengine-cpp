#define GLFW_INCLUDE_VULKAN
#include "Window.hpp"
#include <iostream>

Window::Window(std::string title, unsigned int width, unsigned int height)
    : width(width), height(height) {
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW.");

    if (!glfwVulkanSupported())
        throw std::runtime_error("GLFW failed to find the Vulkan loader");

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int newWidth, int newHeight)
        {
            if (newWidth <= 0 || newHeight <= 0) {
                return;
            }

            auto myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

            myWindow->width = newWidth;
            myWindow->height = newHeight;

            // need to parallelize
            for (auto l : myWindow->resizeListeners)
                l(window, newWidth, newHeight);
        });
}

void Window::awaitEventsLoop() {
    glfwShowWindow(window);
    while (!glfwWindowShouldClose(window))
        glfwWaitEvents();
}

void Window::registerResizeListener(const WindowResizeListener& listener) {
    resizeListeners.push_back(listener);
}

void Window::createSurface(VkInstance vkInstance, VkSurfaceKHR& surface)
{
    VKCHECK(glfwCreateWindowSurface(vkInstance, getWindow(), nullptr, &surface),
        "Failed to create window surface.");
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}