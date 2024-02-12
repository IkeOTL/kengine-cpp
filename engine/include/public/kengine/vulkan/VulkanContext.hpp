#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VmaInclude.hpp>
#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/QueueFamilies.hpp>
#include <kengine/vulkan/VulkanQueue.hpp>
#include <kengine/vulkan/Swapchain.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/Window.hpp>
#include <kengine/vulkan/CommandBuffer.hpp>
#include <kengine/vulkan/CommandPool.hpp>
#include <kengine/vulkan/QueueOwnerTransfer.hpp>
#include <kengine/vulkan/pipelines/PipelineCache.hpp>

#include <glm/vec2.hpp>
#include <functional>
#include <memory>
#include <queue>

class SamplerCache;
class VulkanContext;
class RenderPass;
class RenderPassContext;


class GpuUploadable {
private:
    std::unique_ptr<GpuBuffer> gpuBuffer;

public:
    virtual void upload(VulkanContext& vkCxt, void* data) = 0;
    virtual VkDeviceSize size() = 0;

    void setGpuBuffer(std::unique_ptr<GpuBuffer>&& buf) {
        gpuBuffer = std::move(buf);
    }

    GpuBuffer* getGpuBuffer() const {
        return gpuBuffer.get();
    }
};

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

struct RenderFrameContext {
    const int frameIndex;
    const glm::uvec2 swapchainExtents;
    const int swapchainIndex;
    const VkSemaphore imageSemaphore;
    VkSemaphore cullComputeSemaphore = VK_NULL_HANDLE;
    const VkFence fence;
    const VkCommandBuffer cmd;
};

class VulkanContext {

public:
    static const size_t FRAME_OVERLAP = 3;

    using RenderPassCreator = std::function<std::vector<std::unique_ptr<RenderPass>>(VkDevice, ColorFormatAndSpace&)>;
    using CommandBufferRecordFunc = std::function<std::function<void()>(const CommandBuffer&)>;

    VulkanContext(RenderPassCreator&& renderPassCreator, SwapchainCreator::OnSwapchainCreate&& onSwapchainCreate);
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

    void setSwapchain(std::unique_ptr<Swapchain>&& sc) {
        swapchain = std::move(sc);
    }

    ColorFormatAndSpace& getColorFormatAndSpace() {
        return colorFormatAndSpace;
    }

    std::vector<std::unique_ptr<RenderPass>>& getRenderPasses() {
        return renderPasses;
    }

    void beginRenderPass(RenderPassContext& rpCxt);
    void endRenderPass(RenderPassContext& rpCxt);

    std::unique_ptr<GpuBuffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags,
        VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) const;

    VkDeviceSize alignUboFrame(VkDeviceSize baseFrameSize) const;
    VkDeviceSize alignSsboFrame(VkDeviceSize baseFrameSize) const;

    void uploadBuffer(GpuUploadable& obj, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
        VkBufferUsageFlags usageFlags, std::function<void(VkCommandBuffer)> beforeSubmit);
    void uploadBuffer(GpuUploadable& obj, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
        VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocFlags, std::function<void(VkCommandBuffer)> beforeSubmit);

    void recordAndSubmitTransferCmdBuf(CommandBufferRecordFunc func, bool awaitFence);

    void recordAndSubmitCmdBuf(std::unique_ptr<CommandBuffer>&& cmd, VulkanQueue& queue, CommandBufferRecordFunc func, bool awaitFence);

    uint32_t getGfxQueueFamilyIndex() {
        return gfxQueueFamilyIndex;
    }

    uint32_t getCompQueueFamilyIndex() {
        return compQueueFamilyIndex;
    }

    uint32_t getXferQueueFamilyIndex() {
        return xferQueueFamilyIndex;
    }

    CommandPool* getCommandPool() {
        return commandPool.get();
    }

    PipelineCache& getPipelineCache() {
        return pipelineCache;
    }

    VulkanQueue& getComputeQueue() {
        return *computeQueue;
    }

    VmaAllocator getVmaAllocator() {
        return vmaAllocator;
    }

    SamplerCache& getSamplerCache();

    void submitQueueTransfer(std::shared_ptr<QueueOwnerTransfer> qXfer);

private:
    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT debugCallbackHandle = VK_NULL_HANDLE;

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    ColorFormatAndSpace colorFormatAndSpace{};
    VkPhysicalDeviceProperties2 vkPhysicalDeviceProps{};
    VkPhysicalDeviceMemoryProperties2 vkPhysicalDeviceMemoryProps{};
    QueueFamilies queueFamilies{};
    VkDevice vkDevice = VK_NULL_HANDLE;
    std::unique_ptr<Swapchain> swapchain;

    VmaVulkanFunctions vmaVkFunctions;
    VmaAllocator vmaAllocator = VK_NULL_HANDLE;

    uint32_t gfxQueueFamilyIndex;
    uint32_t compQueueFamilyIndex;
    uint32_t xferQueueFamilyIndex;

    std::shared_ptr<VulkanQueue> graphicsQueue;
    std::shared_ptr<VulkanQueue> computeQueue;
    std::shared_ptr<VulkanQueue> transferQueue;

    std::vector<std::unique_ptr<RenderPass>> renderPasses;

    RenderPassCreator renderPassCreator;
    SwapchainCreator swapchainCreator;

    std::unique_ptr<CommandPool> commandPool;
    std::vector<std::unique_ptr<CommandBuffer>> frameCmdBufs;

    mutable std::mutex qXferMtx;
    std::queue<std::shared_ptr<QueueOwnerTransfer>> vkQueueTransfers;

    mutable std::mutex waitingFenceMtx;
    std::unordered_map<VkFence, std::function<void()>> vkFenceActions;
    std::unique_ptr<GpuBufferCache> gpuBufferCache;

    std::unique_ptr<SamplerCache> samplerCache;
    PipelineCache pipelineCache{};

    void createVkInstance(bool validationOn);
    void setupDebugging();
    void grabFirstPhysicalDevice();
    void createDevice();
    void createQueues();
    void createVmaAllocator();
};