#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/input/InputManager.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <functional>

class Window {
public:
    using WindowResizeListener = std::function<void(GLFWwindow*, int, int)>;

private:
    GLFWwindow* window;
    unsigned int width, height;
    uint64_t lastMouseMove = 0L;

    InputManager* inputManager = nullptr;

    std::vector<WindowResizeListener*> resizeListeners;

public:
    Window(std::string title, unsigned int width, unsigned int height);
    ~Window();

    static inline std::unique_ptr<Window> create(std::string title, unsigned int width, unsigned int height) {
        return std::make_unique<Window>(title, width, height);
    }

    void createSurface(VkInstance vkInstance, VkSurfaceKHR& surface);
    void registerResizeListener(WindowResizeListener* listener);

    void pollEvents();

    GLFWwindow* getWindow() {
        return window;
    }

    void setInputManager(InputManager* im) {
        inputManager = im;
    }

    unsigned int getWidth() const {
        return width;
    }

    unsigned int getHeight() const {
        return height;
    }
};