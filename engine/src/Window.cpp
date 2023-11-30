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