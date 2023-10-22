#pragma once
#include <Window.hpp>
#include <vulkan/vulkan.h>
#include <memory>
#include <ColorFormatAndSpace.hpp>

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    void init(Window& window, bool validationOn);
    void createVkInstance(bool validationOn);
    void setupDebugging();
    void grabFirstPhysicalDevice();

private:
    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT debugCallbackHandle = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physicalDeviceProps{};
    ColorFormatAndSpace colorFormatAndSpace{};
};