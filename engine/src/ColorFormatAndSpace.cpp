#define GLFW_INCLUDE_VULKAN
#include "Window.hpp"
#include <iostream>

Window::Window() {
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW.");

    if (!glfwVulkanSupported())
        throw std::runtime_error("GLFW failed to find the Vulkan loader");

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(1920, 1080, "Demo", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwShowWindow(window);
}

void Window::createSurface(VkInstance vkInstance, VkSurfaceKHR& surface)
{
    auto code = glfwCreateWindowSurface(vkInstance, getWindow(), nullptr, &surface);
    if (code != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface.");
}

void Window::pollInput() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}