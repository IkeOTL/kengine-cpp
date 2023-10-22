#pragma once
#include <Window.hpp>
#include <vulkan/vulkan.h>

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    void init(Window& window, bool validationOn);

private:
    VkInstance vkInstance;
};