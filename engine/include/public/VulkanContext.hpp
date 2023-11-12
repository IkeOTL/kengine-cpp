#pragma once

#include <VulkanInclude.hpp>
#include "VmaInclude.hpp"

#include <Window.hpp>
#include <memory>
#include <ColorFormatAndSpace.hpp>
#include <QueueFamilies.hpp>
#include <VulkanQueue.hpp>
#include <glm/vec2.hpp>
#include <functional>

class RenderPass;

class VulkanContext {

public:
    VulkanContext();
    ~VulkanContext();

    void init(Window& window, bool validationOn);

    struct RenderFrameContext {
        const int frameIndex;
        const glm::ivec2 swapchainExtents;
        const int swapchainIndex;
        const VkSemaphore imageSemaphore;
        VkSemaphore cullComputeSemaphore = VK_NULL_HANDLE;
        const VkFence fence;
        const VkCommandBuffer cmd;
    };

    //using RenderPassCreator = std::function<RenderPass(int, int)>;

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

    std::shared_ptr<VulkanQueue> graphicsQueue;
    std::shared_ptr<VulkanQueue> computeQueue;
    std::shared_ptr<VulkanQueue> transferQueue;

    std::vector<std::unique_ptr<RenderPass>> renderPasses;

    void createVkInstance(bool validationOn);
    void setupDebugging();
    void grabFirstPhysicalDevice();
    void createDevice();
    void createQueues();
    void createVmaAllocator();
};