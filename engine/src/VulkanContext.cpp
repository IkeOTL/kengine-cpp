#include "VulkanContext.hpp"
#include <iostream>

void VulkanContext::init(Window& window, bool validationOn) {
    auto glfwExtensionCount = 0u;
    auto glfwReqExts = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; i++) {
        printf("GLFW Extension: %s\n", glfwReqExts[i]);
    }
}