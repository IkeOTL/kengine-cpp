#pragma once

#include <vulkan/vulkan.h>

//#define VMA_VULKAN_VERSION 1003000
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#include <Window.hpp>
#include <memory>
#include <ColorFormatAndSpace.hpp>
#include <QueueFamilies.hpp>
#include <VulkanQueue.hpp>

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    void init(Window& window, bool validationOn);
    VkResult queueSubmit(VkQueue queue, unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence);

private:
    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT debugCallbackHandle = VK_NULL_HANDLE;

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties2 vkPhysicalDeviceProps{};
    ColorFormatAndSpace colorFormatAndSpace{};
    VkPhysicalDeviceMemoryProperties2 vkPhysicalDeviceMemoryProps{};
    QueueFamilies queueFamilies{};
    VkDevice vkDevice = VK_NULL_HANDLE;

    VmaVulkanFunctions vmaVkFunctions;
    VmaAllocator vmaAllocator = VK_NULL_HANDLE;

    unsigned int gfxQueueFamilyIndex;
    unsigned int compQueueFamilyIndex;
    unsigned int xferQueueFamilyIndex;

    VulkanQueue* graphicsQueue;
    VulkanQueue* computeQueue;
    VulkanQueue* transferQueue;

    void createVkInstance(bool validationOn);
    void setupDebugging();
    void grabFirstPhysicalDevice();
    void createDevice();
    void createQueues();
    void createVmaAllocator();
};