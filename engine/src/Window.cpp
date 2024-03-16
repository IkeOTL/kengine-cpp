#define GLFW_INCLUDE_VULKAN
#include <kengine/Window.hpp>
#include <iostream>
#include <chrono>

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
            if (newWidth <= 0 || newHeight <= 0)
                return;

            auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

            myWindow->width = newWidth;
            myWindow->height = newHeight;

            // need to parallelize
            for (auto* l : myWindow->resizeListeners)
                (*l)(window, newWidth, newHeight);
        });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
        {
            auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

            if (!myWindow->inputManager)
                return;

            auto now = std::chrono::high_resolution_clock::now();
            auto nowNano = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

            if (nowNano - myWindow->lastMouseMove >= 1000000) {
                myWindow->inputManager->onMoveEvent(window, xpos, ypos);
                myWindow->lastMouseMove = nowNano;
            }
        });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (myWindow->inputManager)
                return;

            myWindow->inputManager->onButtonEvent(window, button, action, mods);
        });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

            if (myWindow->inputManager)
                return;

            if (action == GLFW_REPEAT)
                return;

            myWindow->inputManager->onKeyEvent(window, key, scancode, action, mods);
        });

    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
        {
            auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

            if (myWindow->inputManager)
                return;

            myWindow->inputManager->onCharEvent(window, codepoint);
        });
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::registerResizeListener(WindowResizeListener* listener) {
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