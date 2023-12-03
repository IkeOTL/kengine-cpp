#pragma once
#pragma once

#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VmaInclude.hpp>
#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/QueueFamilies.hpp>
#include <kengine/vulkan/VulkanQueue.hpp>
#include <kengine/Window.hpp>

#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/Swapchain.hpp>

#include <glm/vec2.hpp>
#include <functional>
#include <memory>

class VulkanContext;

class SwapchainCreator {
public:
    using OnSwapchainCreate = std::function<void(VulkanContext&, Swapchain&, std::vector<std::unique_ptr<RenderPass>>&)>;

    SwapchainCreator(OnSwapchainCreate onSwapchainCreate)
        : onSwapchainCreate(onSwapchainCreate) {}

    void init(Window& window);

    void setMustRecreate(bool mustRecreate) {
        this->mustRecreate = mustRecreate;
    }

    bool recreate(VulkanContext& vkCxt, bool force, Swapchain& oldSwapchain, OnSwapchainCreate& cb);

private:
    std::mutex lock{};
    int targetWidth = 0, targetHeight = 0;
    bool mustRecreate = false;
    OnSwapchainCreate onSwapchainCreate;
};

class VulkanContext {

public:
    static const size_t FRAME_OVERLAP = 3;

    using RenderPassCreator = std::function<std::vector<std::unique_ptr<RenderPass>>(VkDevice, ColorFormatAndSpace&)>;


    VulkanContext(RenderPassCreator renderPassCreator, SwapchainCreator::OnSwapchainCreate onSwapchainCreate)
        : renderPassCreator(std::move(renderPassCreator)),
        swapchainCreator(std::move(onSwapchainCreate)) {}
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VulkanContext(VulkanContext&&) = default;
    VulkanContext& operator=(VulkanContext&&) = default;

    void init(Window& window, bool validationOn);

    VkSurfaceKHR getVkSurface() {
        return vkSurface;
    }

    VkDevice getVkDevice() {
        return vkDevice;
    }

    VkPhysicalDevice getVkPhysicalDevice() {
        return vkPhysicalDevice;
    }

    Swapchain* getSwapchain() {
        return swapchain.get();
    }

    void setSwapchain(std::unique_ptr<Swapchain> sc) {
        swapchain = std::move(sc);
    }

    ColorFormatAndSpace& getColorFormatAndSpace() {
        return colorFormatAndSpace;
    }

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
    SwapchainCreator swapchainCreator;

    void createVkInstance(bool validationOn);
    void setupDebugging();
    void grabFirstPhysicalDevice();
    void createDevice();
    void createQueues();
    void createVmaAllocator();
};