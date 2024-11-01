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
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include "VulkanObject.hpp"

#include <glm/vec2.hpp>
#include <functional>
#include <memory>
#include <queue>
#include <kengine/DebugContext.hpp>
#include "VulkanObject.hpp"

namespace ke {
    class SamplerCache;
    class VulkanContext;
    class RenderPass;
    class RenderPassContext;
    class DescriptorSetLayoutCache;

    class FrameSyncObjects;

    class SwapchainCreator {
    public:
        using OnSwapchainCreate = std::function<void(VulkanContext&, Swapchain&, std::vector<std::unique_ptr<RenderPass>>&)>;

        SwapchainCreator(Window& window, OnSwapchainCreate&& onSwapchainCreate)
            : window(window),
              onSwapchainCreate(std::move(onSwapchainCreate)) {}

        void init();

        void setMustRecreate(bool mustRecreate) {
            this->mustRecreate = mustRecreate;
        }

        bool recreate(VulkanContext& vkCxt, bool force, Swapchain& oldSwapchain);

    private:
        Window& window;
        std::mutex lock{};
        int targetWidth = 0, targetHeight = 0;
        bool mustRecreate = true;
        std::unique_ptr<Window::WindowResizeListener> windowResizeListener;
        OnSwapchainCreate onSwapchainCreate;
    };

    struct RenderFrameContext {
        const uint32_t frameIndex;
        const glm::uvec2 swapchainExtents;
        const uint32_t swapchainIndex;
        const VkSemaphore imageSemaphore;
        VkSemaphore cullComputeSemaphore = VK_NULL_HANDLE;
        const VkFence fence;
        const VkCommandBuffer cmd;
    };

    class VulkanContext {
    public:
        static const uint32_t FRAME_OVERLAP = 3;

        using RenderPassCreator = std::function<std::vector<std::unique_ptr<RenderPass>>(VkDevice, ColorFormatAndSpace&)>;
        using PipelineCacheCreator = std::function<std::unique_ptr<PipelineCache>(VulkanContext& vkCtx, std::vector<std::unique_ptr<RenderPass>>& rp)>;
        using CommandBufferRecordFunc = std::function<std::function<void()>(const CommandBuffer&)>;

        VulkanContext(Window& window, RenderPassCreator&& renderPassCreator, PipelineCacheCreator&& pipelineCacheCreator, SwapchainCreator::OnSwapchainCreate&& onSwapchainCreate);
        ~VulkanContext();

        static inline std::unique_ptr<VulkanContext> create(Window& window, RenderPassCreator&& renderPassCreator, PipelineCacheCreator&& pipelineCacheCreator, SwapchainCreator::OnSwapchainCreate&& onSwapchainCreate) {
            return std::make_unique<VulkanContext>(window, std::move(renderPassCreator), std::move(pipelineCacheCreator), std::move(onSwapchainCreate));
        }

        VulkanContext(const VulkanContext&) = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;

        VulkanContext(VulkanContext&&) = default;
        VulkanContext& operator=(VulkanContext&&) = default;

        void init(bool validationOn);

        VkDevice getVkDevice() {
            return vkDevice->handle;
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

        DescriptorSetAllocators& getDescSetAllocators() {
            return descSetAllocators;
        }

        template <typename R>
        R& getRenderPass(int i) {
            return static_cast<R&>(*renderPasses[i]);
        }

        RenderPass& getRenderPass(int i);

        std::unique_ptr<RenderFrameContext> createNextFrameContext();
        void renderBegin(RenderFrameContext& cxt);
        void renderEnd(RenderFrameContext& cxt);

        void beginRenderPass(RenderPassContext& rpCxt);
        void endRenderPass(RenderPassContext& rpCxt);

        std::unique_ptr<GpuBuffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags,
            VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) const;

        VkDeviceSize alignUboFrame(VkDeviceSize baseFrameSize) const;
        VkDeviceSize alignSsboFrame(VkDeviceSize baseFrameSize) const;

        std::unique_ptr<GpuBuffer> uploadBuffer(std::function<void(VulkanContext& vkCxt, void* data)> dataProvider, VkDeviceSize dstBufSize,
            VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkBufferUsageFlags usageFlags, std::function<void(VkCommandBuffer)> beforeSubmit);
        std::unique_ptr<GpuBuffer> uploadBuffer(std::function<void(VulkanContext& vkCxt, void* data)> dataProvider, VkDeviceSize dstBufSize,
            VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocFlags, std::function<void(VkCommandBuffer)> beforeSubmit);

        void recordAndSubmitTransferCmdBuf(CommandBufferRecordFunc func, bool awaitFence);

        void recordAndSubmitCmdBuf(std::unique_ptr<CommandBuffer>&& cmd, VulkanQueue& queue, CommandBufferRecordFunc func, bool awaitFence);

        uint32_t getGfxQueueFamilyIndex() const {
            return gfxQueueFamilyIndex;
        }

        uint32_t getCompQueueFamilyIndex() const {
            return compQueueFamilyIndex;
        }

        uint32_t getXferQueueFamilyIndex() const {
            return xferQueueFamilyIndex;
        }

        CommandPool* getCommandPool() {
            return commandPool.get();
        }

        PipelineCache& getPipelineCache() {
            return *pipelineCache;
        }

        DescriptorSetLayoutCache& getDescSetLayoutCache();

        VulkanQueue& getComputeQueue() {
            return *computeQueue;
        }

        VulkanQueue& getGraphicsQueue() {
            return *graphicsQueue;
        }

        VmaAllocator getVmaAllocator() const {
            return vulkanAllocator->handle;
        }

        SamplerCache& getSamplerCache();

        VkInstance getVkInstance() const {
            return vulkanInstance->handle;
        }

        VkSurfaceKHR getVkSurface() {
            return vkSurface->handle;
        }

        void setDebugContext(DebugContext* d) {
            debugContext = d;
        }

        DebugContext* getDebugContext() {
            return debugContext;
        }

        GpuBufferCache& getGpuBufferCache() {
            return *gpuBufferCache;
        }

        void submitQueueTransfer(std::shared_ptr<QueueOwnerTransfer> qXfer);

    private:
        std::unique_ptr<ke::VulkanInstance> vulkanInstance;
        std::unique_ptr<ke::VulkanSurface> vkSurface = nullptr;
        Window& window;
        VkDebugReportCallbackEXT debugCallbackHandle = VK_NULL_HANDLE;

        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        ColorFormatAndSpace colorFormatAndSpace{};
        VkPhysicalDeviceProperties2 vkPhysicalDeviceProps{};
        VkPhysicalDeviceMemoryProperties2 vkPhysicalDeviceMemoryProps{};
        QueueFamilies queueFamilies{};
        std::unique_ptr<ke::VulkanDevice> vkDevice;
        std::unique_ptr<Swapchain> swapchain;

        VmaVulkanFunctions vmaVkFunctions{};
        std::unique_ptr<ke::VulkanAllocator> vulkanAllocator;

        uint32_t gfxQueueFamilyIndex = 0;
        uint32_t compQueueFamilyIndex = 0;
        uint32_t xferQueueFamilyIndex = 0;

        std::shared_ptr<VulkanQueue> graphicsQueue;
        std::shared_ptr<VulkanQueue> computeQueue;
        std::shared_ptr<VulkanQueue> transferQueue;

        std::vector<std::unique_ptr<RenderPass>> renderPasses;

        RenderPassCreator renderPassCreator;
        PipelineCacheCreator pipelineCacheCreator;
        SwapchainCreator swapchainCreator;

        std::unique_ptr<CommandPool> commandPool;
        std::vector<std::unique_ptr<CommandBuffer>> frameCmdBufs;

        std::unique_ptr<FrameSyncObjects> frameSync;

        mutable std::mutex qXferMtx{};
        std::queue<std::shared_ptr<QueueOwnerTransfer>> vkQueueTransfers;

        mutable std::mutex waitingFenceMtx{};
        std::unordered_map<std::unique_ptr<ke::VulkanFence>, std::function<void()>> vkFenceActions;
        std::unique_ptr<GpuBufferCache> gpuBufferCache;

        std::unique_ptr<PipelineCache> pipelineCache;
        std::unique_ptr<SamplerCache> samplerCache;
        std::unique_ptr<DescriptorSetLayoutCache> descSetLayoutCache;

        DescriptorSetAllocators descSetAllocators;

        uint64_t frameNumber = 0;

        DebugContext* debugContext = nullptr;

        void createVkInstance(bool validationOn);
        void setupDebugging();
        void grabFirstPhysicalDevice();
        void createDevice();
        void createQueues();
        void createVmaAllocator();

        uint32_t acquireImage(uint32_t& pImageIndex);
        void processFinishedFences();
    };

    class FrameSyncObjects {
    private:
        const VkDevice vkDevice;

        std::unique_ptr<ke::VulkanFence> frameFences[VulkanContext::FRAME_OVERLAP];
        std::unique_ptr<ke::VulkanSemaphore> frameSemaphores[VulkanContext::FRAME_OVERLAP];
        std::unique_ptr<ke::VulkanSemaphore> imageAcquireSemaphores[VulkanContext::FRAME_OVERLAP];

        void createFences(VkDevice device, std::unique_ptr<ke::VulkanFence>* fences);
        void createSemaphores(VkDevice device, std::unique_ptr<ke::VulkanSemaphore>* semaphores);

    public:
        FrameSyncObjects(VkDevice vkDevice)
            : vkDevice(vkDevice) {}

        ~FrameSyncObjects() = default;

        void init();

        VkFence getFrameFence(uint32_t i) const {
            return frameFences[i]->handle;
        }

        VkSemaphore getFrameSemaphore(uint32_t i) const {
            return frameSemaphores[i]->handle;
        }

        VkSemaphore getImageAcquireSemaphore(uint32_t i) const {
            return imageAcquireSemaphores[i]->handle;
        }
    };
} // namespace ke