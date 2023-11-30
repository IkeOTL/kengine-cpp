#pragma once

#include <VulkanInclude.hpp>
#include <VmaInclude.hpp>

#include <Window.hpp>
#include <memory>
#include <ColorFormatAndSpace.hpp>
#include <QueueFamilies.hpp>
#include <VulkanQueue.hpp>
#include <glm/vec2.hpp>
#include <functional>

#include <renderpass/RenderPass.hpp>
#include <Swapchain.hpp>

class VulkanContext {

public:
    static const size_t FRAME_OVERLAP = 3;

    using RenderPassCreator = std::function<std::vector<std::unique_ptr<RenderPass>>(VkDevice, ColorFormatAndSpace&)>;
    using OnSwapchainCreate = std::function<void(VulkanContext&, Swapchain&, std::vector<std::unique_ptr<RenderPass>>&)>;

    VulkanContext(RenderPassCreator renderPassCreator, OnSwapchainCreate onSwapchainCreate)
        : renderPassCreator(std::move(renderPassCreator)),
        onSwapchainCreate(onSwapchainCreate) {}
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VulkanContext(VulkanContext&&) = default;
    VulkanContext& operator=(VulkanContext&&) = default;

    void init(Window& window, bool validationOn);

    std::vector<std::unique_ptr<RenderPass>>& getRenderPasses() {
        return renderPasses;
    }

    struct RenderFrameContext {
        const int frameIndex;
        const glm::uvec2 swapchainExtents;
        const int swapchainIndex;
        const VkSemaphore imageSemaphore;
        VkSemaphore cullComputeSemaphore = VK_NULL_HANDLE;
        const VkFence fence;
        const VkCommandBuffer cmd;
    };

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
    std::unique_ptr<Swapchain> swapchain;

    VmaVulkanFunctions vmaVkFunctions;
    VmaAllocator vmaAllocator = VK_NULL_HANDLE;

    unsigned int gfxQueueFamilyIndex;
    unsigned int compQueueFamilyIndex;
    unsigned int xferQueueFamilyIndex;

    std::shared_ptr<VulkanQueue> graphicsQueue;
    std::shared_ptr<VulkanQueue> computeQueue;
    std::shared_ptr<VulkanQueue> transferQueue;

    std::vector<std::unique_ptr<RenderPass>> renderPasses;

    RenderPassCreator renderPassCreator;
    OnSwapchainCreate onSwapchainCreate;

    void createVkInstance(bool validationOn);
    void setupDebugging();
    void grabFirstPhysicalDevice();
    void createDevice();
    void createQueues();
    void createVmaAllocator();
};