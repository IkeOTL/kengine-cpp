#define VMA_IMPLEMENTATION
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/vulkan/SamplerCache.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <kengine/Logger.hpp>
#include <tracy/Tracy.hpp>

namespace ke {

    VulkanContext::VulkanContext(Window& window, RenderPassCreator&& renderPassCreator, PipelineCacheCreator&& pipelineCacheCreator, SwapchainCreator::OnSwapchainCreate&& onSwapchainCreate)
        : window(window),
          renderPassCreator(std::move(renderPassCreator)),
          pipelineCacheCreator(std::move(pipelineCacheCreator)),
          swapchainCreator(SwapchainCreator(window, std::move(onSwapchainCreate))) {}

    VulkanContext::~VulkanContext() {
        // print VMA stats
        //{
        //    char* statsString = nullptr;
        //    vmaBuildStatsString(vmaAllocator, &statsString, true);
        //    KE_LOG_INFO(statsString);
        //    vmaFreeStatsString(vmaAllocator, statsString);
        //}

        auto funcDestroyDebug = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance->handle, "vkDestroyDebugReportCallbackEXT");
        if (funcDestroyDebug)
            funcDestroyDebug(vulkanInstance->handle, debugCallbackHandle, nullptr);
    }

    void VulkanContext::init(bool validationOn) {
        createVkInstance(validationOn);

        if (validationOn)
            setupDebugging();

        vkSurface = window.createSurface(vulkanInstance->handle);
        grabFirstPhysicalDevice();
        colorFormatAndSpace.init(vkPhysicalDevice, vkSurface->handle);

        queueFamilies.init(vkPhysicalDevice);
        gfxQueueFamilyIndex = queueFamilies.getGfxCompXferQueues()[0];
        compQueueFamilyIndex = gfxQueueFamilyIndex;
        xferQueueFamilyIndex = queueFamilies.getTransferQueues()[0];

        createDevice();
        createQueues();
        createVmaAllocator();

        auto vkDevice = this->vkDevice->handle;

        frameSync = std::make_unique<FrameSyncObjects>(vkDevice);
        frameSync->init();

        swapchain = Swapchain(vkDevice).replace(vkPhysicalDevice, vkDevice, window.getWidth(), window.getHeight(), vkSurface->handle, colorFormatAndSpace);

        swapchainCreator.init();

        // todo: where should this be initiated? need to try to get this into render thread
        commandPool = std::make_unique<CommandPool>(vkDevice);
        commandPool->initThread(*this);
        for (auto i = 0; i < FRAME_OVERLAP; i++)
            frameCmdBufs.push_back(commandPool->createGraphicsCmdBuf());

        renderPasses = std::move(renderPassCreator(vkDevice, colorFormatAndSpace));
        for (auto& rp : renderPasses)
            rp->init(*this);

        samplerCache = std::make_unique<SamplerCache>(*this);
        gpuBufferCache = std::make_unique<GpuBufferCache>(*this);
        descSetLayoutCache = std::make_unique<DescriptorSetLayoutCache>(*this);
        pipelineCache = pipelineCacheCreator(*this, renderPasses);

        for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            auto ptr = std::make_unique<DescriptorSetAllocator>(vkDevice, *descSetLayoutCache);
            ptr->init();
            descSetAllocators[i] = std::move(ptr);
        }
    }

    /// <summary>
    /// Returns the current frameIndex and updates the swapchain image index reference
    /// </summary>
    uint32_t VulkanContext::acquireImage(uint32_t& pImageIndex) {
        if (swapchainCreator.recreate(*this, false, *swapchain))
            frameNumber = 0;

        auto idx = static_cast<uint32_t>(frameNumber % FRAME_OVERLAP);

        auto vkDevice = this->vkDevice->handle;

        auto fence = frameSync->getFrameFence(idx);
        vkWaitForFences(vkDevice, 1, &fence, true, 0x7fffffffffffffffL);
        vkResetFences(vkDevice, 1, &fence);

        auto imgSemaphore = frameSync->getImageAcquireSemaphore(idx);
        auto imgRes = vkAcquireNextImageKHR(vkDevice, swapchain->getSwapchain(), -1L, imgSemaphore, VK_NULL_HANDLE, &pImageIndex);

        while (imgRes == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchainCreator.recreate(*this, true, *swapchain);

            idx = 0;
            frameNumber = 0;

            fence = frameSync->getFrameFence(idx);
            vkWaitForFences(vkDevice, 1, &fence, true, 0x7fffffffffffffffL);
            vkResetFences(vkDevice, 1, &fence);

            imgSemaphore = frameSync->getImageAcquireSemaphore(idx);
            imgRes = vkAcquireNextImageKHR(vkDevice, swapchain->getSwapchain(), -1L, imgSemaphore, VK_NULL_HANDLE, &pImageIndex);
        }

        return idx;
    }

    std::unique_ptr<RenderFrameContext> VulkanContext::createNextFrameContext() {
        ZoneScoped;
        uint32_t pImageIndex;
        auto idx = acquireImage(pImageIndex);
        auto cmd = frameCmdBufs[pImageIndex]->vkCmdBuf;
        auto imgSemaphore = frameSync->getImageAcquireSemaphore(idx);
        auto fence = frameSync->getFrameFence(idx);

        return std::make_unique<RenderFrameContext>(RenderFrameContext{
            idx,
            swapchain->getExtents(),
            pImageIndex,
            imgSemaphore,
            VK_NULL_HANDLE,
            fence,
            cmd});
    }

    void VulkanContext::renderBegin(RenderFrameContext& cxt) {
        ZoneScoped;

        VkCommandBufferBeginInfo cmdBeginInfo{};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VKCHECK(vkBeginCommandBuffer(cxt.cmd, &cmdBeginInfo),
            "Command buffer failed to do the thing.");

        std::lock_guard<std::mutex> lock(qXferMtx);
        while (!vkQueueTransfers.empty()) {
            auto& xfer = vkQueueTransfers.front();
            xfer->applyAcquireBarrier(cxt.cmd);
            vkQueueTransfers.pop();
        }
    }

    void VulkanContext::renderEnd(RenderFrameContext& cxt) {
        ZoneScoped;

        vkEndCommandBuffer(cxt.cmd);

        auto frameSemaphore = frameSync->getFrameSemaphore(cxt.frameIndex);

        {
            auto sCount = cxt.cullComputeSemaphore ? 2 : 1;

            std::vector<VkSemaphoreSubmitInfo> sumbitSemaInfo(sCount);

            sumbitSemaInfo[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            sumbitSemaInfo[0].semaphore = cxt.imageSemaphore;
            sumbitSemaInfo[0].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

            if (cxt.cullComputeSemaphore) {
                sumbitSemaInfo[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                sumbitSemaInfo[1].semaphore = cxt.cullComputeSemaphore;
                sumbitSemaInfo[1].stageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            }

            VkCommandBufferSubmitInfo cmdInfo{};
            cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            cmdInfo.commandBuffer = cxt.cmd;

            VkSemaphoreSubmitInfo presentSemaInfo{};
            presentSemaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            presentSemaInfo.semaphore = frameSemaphore;

            VkSubmitInfo2 submit{};
            submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            submit.commandBufferInfoCount = 1;
            submit.pCommandBufferInfos = &cmdInfo;

            submit.waitSemaphoreInfoCount = sCount;
            submit.pWaitSemaphoreInfos = sumbitSemaInfo.data();

            submit.signalSemaphoreInfoCount = 1;
            submit.pSignalSemaphoreInfos = &presentSemaInfo;

            graphicsQueue->submit(1, &submit, cxt.fence);
        }

        {
            auto swapchainHandle = swapchain->getSwapchain();
            auto swapchainImgIdx = cxt.swapchainIndex;

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &frameSemaphore;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchainHandle;
            presentInfo.pImageIndices = &swapchainImgIdx;

            auto result = graphicsQueue->present(&presentInfo);

            swapchainCreator.setMustRecreate(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR);
        }

        processFinishedFences();
        frameNumber++;
    }

    void VulkanContext::beginRenderPass(RenderPassContext& rpCxt) {
        renderPasses[rpCxt.renderPassIndex]->begin(rpCxt);
    }

    void VulkanContext::endRenderPass(RenderPassContext& rpCxt) {
        renderPasses[rpCxt.renderPassIndex]->end(rpCxt);
    }

    void VulkanContext::processFinishedFences() {
        for (auto it = vkFenceActions.begin(); it != vkFenceActions.end();) {
            auto& fence = it->first;

            // Check the status of the fence
            if (vkGetFenceStatus(vkDevice->handle, fence->handle) != VK_SUCCESS) {
                ++it;
                continue;
            }

            // Fence is finished; destroy it and run the associated action
            // vkDestroyFence(vkDevice, fence, nullptr);

            try {
                it->second(); // Execute the action associated with the fence
            } catch (const std::runtime_error& e) {
                KE_LOG_ERROR(std::format("Failed execting fence action: {}", e.what()));
            } catch (...) {
                KE_LOG_ERROR(std::format("Failed execting fence action"));
            }

            // this should erase the fence
            it = vkFenceActions.erase(it);
        }
    }

    void VulkanContext::createVkInstance(bool validationOn) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Demo";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "kengine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // must be done here to keep stuff in scope until vkInstance created
        // apply extensions
        auto glfwExtCnt = 0u;
        auto glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCnt);

        const char* myExts[] = {
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
        const auto myExtCnt = sizeof(myExts) / sizeof(myExts[0]);

        auto allExts = std::vector<const char*>(glfwExts, glfwExts + glfwExtCnt);
        allExts.insert(allExts.end(), myExts, myExts + myExtCnt);

        if (allExts.size()) {
            createInfo.enabledExtensionCount = static_cast<uint32_t>(allExts.size());
            createInfo.ppEnabledExtensionNames = allExts.data();
        }

        // might change this logic since we might need layers that arent validation layers
        if (!validationOn) {
            VkInstance newInstance;
            VKCHECK(vkCreateInstance(&createInfo, nullptr, &newInstance),
                "Failed to create Vulkan instance.");
            vulkanInstance = std::make_unique<ke::VulkanInstance>(newInstance);
            return;
        }

        // apply validation layers
        auto layerCount = 0u;
        VKCHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
            "Failed to get the number of instance layers.");

        auto availableLayers = std::vector<VkLayerProperties>(layerCount);

        VKCHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
            "Failed to get instance layers properties.");

        std::vector<const char*> desiredLayers = {
            "VK_LAYER_LUNARG_standard_validation",
            "VK_LAYER_KHRONOS_validation"};

        std::vector<const char*> layersToEnable;

        // find layers that we want
        for (const auto& layerProperties : availableLayers) {
            if (std::any_of(desiredLayers.begin(), desiredLayers.end(),
                    [&layerProperties](const char* desiredLayerName) {
                        return strcmp(layerProperties.layerName, desiredLayerName) == 0;
                    })) {
                layersToEnable.push_back(layerProperties.layerName);
            }
        }

        for (const auto& layerProperties : layersToEnable)
            KE_LOG_INFO(std::format("Enabled Layer: {}", layerProperties));

        if (layersToEnable.size()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(layersToEnable.size());
            createInfo.ppEnabledLayerNames = layersToEnable.data();
        }

        VkInstance newInstance;
        VKCHECK(vkCreateInstance(&createInfo, nullptr, &newInstance),
            "Failed to create Vulkan instance.");

        vulkanInstance = std::make_unique<ke::VulkanInstance>(newInstance);
    }

    void VulkanContext::setupDebugging() {
        auto flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

        auto vkDebugCbCreateInfo = VkDebugReportCallbackCreateInfoEXT{};
        vkDebugCbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        vkDebugCbCreateInfo.flags = flags;
        vkDebugCbCreateInfo.pfnCallback = +[](VkDebugReportFlagsEXT flags,
                                               VkDebugReportObjectTypeEXT objType,
                                               uint64_t obj,
                                               size_t location,
                                               int32_t code,
                                               const char* layerPrefix,
                                               const char* msg,
                                               void* userData) -> VkBool32 {
            KE_LOG_ERROR(std::format("vkDebug: {}", msg));
            return VK_FALSE;
        };

        auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance->handle, "vkCreateDebugReportCallbackEXT");

        if (!func)
            throw std::runtime_error("Could not get address of vkCreateDebugReportCallbackEXT.");

        if (func(vulkanInstance->handle, &vkDebugCbCreateInfo, nullptr, &debugCallbackHandle) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug callback.");
    }

    void VulkanContext::grabFirstPhysicalDevice() {
        auto deviceCnt = 0u;
        vkEnumeratePhysicalDevices(vulkanInstance->handle, &deviceCnt, VK_NULL_HANDLE);

        if (!deviceCnt)
            throw std::runtime_error("No devices available.");

        std::vector<VkPhysicalDevice> availableDevices(deviceCnt);
        vkEnumeratePhysicalDevices(vulkanInstance->handle, &deviceCnt, availableDevices.data());

        for (const auto& device : availableDevices) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(device, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                vkPhysicalDevice = device;
                break;
            }
        }

        // load up gpu details
        vkPhysicalDeviceProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(vkPhysicalDevice, &vkPhysicalDeviceProps);

        // grab memory props
        vkPhysicalDeviceMemoryProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(vkPhysicalDevice, &vkPhysicalDeviceMemoryProps);
    }

    void VulkanContext::createDevice() {
        auto graphicsQueuePriority = 1.0f;
        auto graphicsQueueCreateInfo = VkDeviceQueueCreateInfo{};
        graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex = gfxQueueFamilyIndex;
        graphicsQueueCreateInfo.queueCount = 1;
        graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

        auto transferQueuePriority = 1.0f;
        auto transferQueueCreateInfo = VkDeviceQueueCreateInfo{};
        transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transferQueueCreateInfo.queueFamilyIndex = xferQueueFamilyIndex;
        transferQueueCreateInfo.queueCount = 1;
        transferQueueCreateInfo.pQueuePriorities = &transferQueuePriority;

        auto features13 = VkPhysicalDeviceVulkan13Features{};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.synchronization2 = true;
        features13.pNext = nullptr;

        auto features12 = VkPhysicalDeviceVulkan12Features{};
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.pNext = &features13;

        auto features11 = VkPhysicalDeviceVulkan11Features{};
        features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        features11.pNext = &features12;

        VkDeviceQueueCreateInfo queueCreateInfos[2] = {graphicsQueueCreateInfo, transferQueueCreateInfo};

        std::vector<const char*> desiredLayers = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            //   VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
        };

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &supportedFeatures);

        if (!supportedFeatures.fillModeNonSolid)
            throw std::runtime_error("Feature not supprted: fillModeNonSolid");

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = VK_TRUE;

        auto createDeviceInfo = VkDeviceCreateInfo{};
        createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createDeviceInfo.pNext = &features11;
        createDeviceInfo.queueCreateInfoCount = 2;
        createDeviceInfo.pQueueCreateInfos = queueCreateInfos;
        createDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(desiredLayers.size());
        createDeviceInfo.ppEnabledExtensionNames = desiredLayers.data();
        createDeviceInfo.pEnabledFeatures = &deviceFeatures;

        VkDevice newVkDevice;
        vkCreateDevice(vkPhysicalDevice, &createDeviceInfo, nullptr, &newVkDevice);
        vkDevice = std::make_unique<ke::VulkanDevice>(newVkDevice);
    }

    void VulkanContext::createQueues() {
        auto vkDevice = this->vkDevice->handle;
        graphicsQueue = std::make_shared<VulkanQueue>(vkDevice, gfxQueueFamilyIndex);
        computeQueue = graphicsQueue;
        transferQueue = std::make_unique<VulkanQueue>(vkDevice, xferQueueFamilyIndex);

        graphicsQueue->init();
        transferQueue->init();
    }

    void VulkanContext::createVmaAllocator() {
        auto vkInstance = vulkanInstance->handle;
        auto vkDevice = this->vkDevice->handle;

        auto allocatorInfo = VmaAllocatorCreateInfo{};
        allocatorInfo.instance = vkInstance;
        allocatorInfo.physicalDevice = vkPhysicalDevice;
        allocatorInfo.device = vkDevice;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

        // required
        vmaVkFunctions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties");
        vmaVkFunctions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties");

        vmaVkFunctions.vkAllocateMemory = (PFN_vkAllocateMemory)vkGetDeviceProcAddr(vkDevice, "vkAllocateMemory");
        vmaVkFunctions.vkFreeMemory = (PFN_vkFreeMemory)vkGetDeviceProcAddr(vkDevice, "vkFreeMemory");
        vmaVkFunctions.vkMapMemory = (PFN_vkMapMemory)vkGetDeviceProcAddr(vkDevice, "vkMapMemory");
        vmaVkFunctions.vkUnmapMemory = (PFN_vkUnmapMemory)vkGetDeviceProcAddr(vkDevice, "vkUnmapMemory");
        vmaVkFunctions.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)vkGetDeviceProcAddr(vkDevice, "vkFlushMappedMemoryRanges");
        vmaVkFunctions.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)vkGetDeviceProcAddr(vkDevice, "vkInvalidateMappedMemoryRanges");
        vmaVkFunctions.vkBindBufferMemory = (PFN_vkBindBufferMemory)vkGetDeviceProcAddr(vkDevice, "vkBindBufferMemory");
        vmaVkFunctions.vkBindImageMemory = (PFN_vkBindImageMemory)vkGetDeviceProcAddr(vkDevice, "vkBindImageMemory");
        vmaVkFunctions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetBufferMemoryRequirements");
        vmaVkFunctions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetImageMemoryRequirements");
        vmaVkFunctions.vkCreateBuffer = (PFN_vkCreateBuffer)vkGetDeviceProcAddr(vkDevice, "vkCreateBuffer");
        vmaVkFunctions.vkDestroyBuffer = (PFN_vkDestroyBuffer)vkGetDeviceProcAddr(vkDevice, "vkDestroyBuffer");
        vmaVkFunctions.vkCreateImage = (PFN_vkCreateImage)vkGetDeviceProcAddr(vkDevice, "vkCreateImage");
        vmaVkFunctions.vkDestroyImage = (PFN_vkDestroyImage)vkGetDeviceProcAddr(vkDevice, "vkDestroyImage");
        vmaVkFunctions.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)vkGetDeviceProcAddr(vkDevice, "vkCmdCopyBuffer");

        // optional
        vmaVkFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties2");

        vmaVkFunctions.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(vkDevice, "vkGetBufferMemoryRequirements2");
        vmaVkFunctions.vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(vkDevice, "vkGetImageMemoryRequirements2");
        vmaVkFunctions.vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(vkDevice, "vkBindBufferMemory2");
        vmaVkFunctions.vkBindImageMemory2KHR = (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(vkDevice, "vkBindImageMemory2");
        vmaVkFunctions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetDeviceBufferMemoryRequirements");
        vmaVkFunctions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetDeviceImageMemoryRequirements");

        allocatorInfo.pVulkanFunctions = &vmaVkFunctions;

        VmaAllocator vmaAllocator;
        VKCHECK(vmaCreateAllocator(&allocatorInfo, &vmaAllocator),
            "Failed to initialize VMA allocator.");

        vulkanAllocator = std::make_unique<ke::VulkanAllocator>(vmaAllocator);
    }

    VkDeviceSize VulkanContext::alignUboFrame(VkDeviceSize baseFrameSize) const {
        auto minUboAlignment = vkPhysicalDeviceProps.properties.limits.minUniformBufferOffsetAlignment;

        if (minUboAlignment <= 0)
            return baseFrameSize;

        return (baseFrameSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    VkDeviceSize VulkanContext::alignSsboFrame(VkDeviceSize baseFrameSize) const {
        auto minSsboAlignment = vkPhysicalDeviceProps.properties.limits.minStorageBufferOffsetAlignment;

        if (minSsboAlignment <= 0)
            return baseFrameSize;

        return (baseFrameSize + minSsboAlignment - 1) & ~(minSsboAlignment - 1);
    }

    void SwapchainCreator::init() {
        windowResizeListener = std::make_unique<Window::WindowResizeListener>([this](GLFWwindow* window, int newWidth, int newHeight) {
            std::lock_guard<std::mutex> lock(this->lock);
            targetWidth = newWidth;
            targetHeight = newHeight;
            setMustRecreate(true);
        });

        window.registerResizeListener(windowResizeListener.get());
    }

    bool SwapchainCreator::recreate(VulkanContext& vkCxt, bool force, Swapchain& oldSwapchain) {
        std::lock_guard<std::mutex> lock(this->lock);

        if (!mustRecreate && !force)
            return false;

        // await device idle
        VKCHECK(vkDeviceWaitIdle(vkCxt.getVkDevice()),
            "Failed to wait for device idle");

        // will destroy swapchain image views
        auto newSwapchain = oldSwapchain.replace(
            vkCxt.getVkPhysicalDevice(),
            vkCxt.getVkDevice(),
            targetWidth,
            targetHeight,
            vkCxt.getVkSurface(),
            vkCxt.getColorFormatAndSpace());

        vkCxt.setSwapchain(std::move(newSwapchain));

        onSwapchainCreate(vkCxt, *vkCxt.getSwapchain(), vkCxt.getRenderPasses());

        return true;
    }

    std::unique_ptr<GpuBuffer> VulkanContext::createBuffer(
        VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) const {
        static std::atomic<uint32_t> runningId = 0;

        VkBufferCreateInfo bufCreateInfo{};
        bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufCreateInfo.size = size;
        bufCreateInfo.usage = usageFlags;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = memoryUsage;
        allocationCreateInfo.flags = allocFlags;

        auto vmaAllocator = getVmaAllocator();

        VkBuffer buffer{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocationInfo{};
        VKCHECK(vmaCreateBuffer(vmaAllocator, &bufCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo),
            "Failed to allocate buffer.");

        VkMemoryPropertyFlags memTypeProperties{};
        vmaGetMemoryTypeProperties(vmaAllocator, allocationInfo.memoryType, &memTypeProperties);
        auto isHostCoherent = (memTypeProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

        return std::make_unique<GpuBuffer>(runningId++, vmaAllocator, buffer, allocation, isHostCoherent);
    }

    void VulkanContext::submitQueueTransfer(std::shared_ptr<QueueOwnerTransfer> qXfer) {
        std::lock_guard<std::mutex> lock(qXferMtx);
        vkQueueTransfers.push(qXfer);
    }

    std::unique_ptr<GpuBuffer> VulkanContext::uploadBuffer(std::function<void(VulkanContext& vkCxt, void* data)> dataProvider, VkDeviceSize dstBufSize,
        const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
        VkBufferUsageFlags usageFlags, std::function<void(VkCommandBuffer)> beforeSubmit) {
        return uploadBuffer(dataProvider, dstBufSize, dstStageMask, dstAccessMask, usageFlags, 0, beforeSubmit);
    }

    std::unique_ptr<GpuBuffer> VulkanContext::uploadBuffer(std::function<void(VulkanContext& vkCxt, void* data)> dataProvider, VkDeviceSize dstBufSize,
        const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask, VkBufferUsageFlags dvcUsageFlags,
        VmaAllocationCreateFlags dvcAllocFlags, std::function<void(VkCommandBuffer)> beforeSubmit) {
        // create staging buffer
        auto stagingBuf = createBuffer(
            dstBufSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

        // copy to staging buffer
        {
            GpuBuffer::ScopedMap scopedMap(*stagingBuf);
            dataProvider(*this, stagingBuf->data());
        }

        // create device buffer
        auto deviceBuf = createBuffer(
            dstBufSize,
            dvcUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            dvcAllocFlags);

        auto cmdBuf = commandPool->createTransferCmdBuf();

        // record and submit to queue, and block until queue work finish
        // share sStagingBuf with lambda so it'll be in respective scopes
        std::shared_ptr<GpuBuffer> sStagingBuf = std::move(stagingBuf);
        recordAndSubmitCmdBuf(
            std::move(cmdBuf),
            *transferQueue,
            [&](const CommandBuffer& cmdBuf) {
                auto qXfer = std::make_shared<BufferQueueOwnerTransfer>(
                    deviceBuf->getVkBuffer(), dstBufSize,
                    xferQueueFamilyIndex, gfxQueueFamilyIndex,
                    dstStageMask, dstAccessMask);
                qXfer->applyReleaseBarrier(cmdBuf.vkCmdBuf);

                VkBufferCopy copy{0, 0, dstBufSize};
                vkCmdCopyBuffer(cmdBuf.vkCmdBuf, sStagingBuf->vkBuffer, deviceBuf->vkBuffer, 1, &copy);

                if (beforeSubmit)
                    beforeSubmit(cmdBuf.vkCmdBuf);

                // executes on fence signaled
                return [this, sStagingBuf, qXfer]() {
                    this->submitQueueTransfer(qXfer);
                };
            },
            true);

        return deviceBuf;
    }

    void VulkanContext::recordAndSubmitTransferCmdBuf(CommandBufferRecordFunc func, bool awaitFence) {
        recordAndSubmitCmdBuf(std::move(commandPool->createTransferCmdBuf()), *transferQueue, func, awaitFence);
    }

    void VulkanContext::recordAndSubmitCmdBuf(std::unique_ptr<CommandBuffer>&& cmd, VulkanQueue& queue, CommandBufferRecordFunc func, bool awaitFence) {
        auto vkDevice = this->vkDevice->handle;

        // record command buffer
        VkCommandBufferBeginInfo cmdBufBeginInfo{};
        cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VKCHECK(vkBeginCommandBuffer(cmd->vkCmdBuf, &cmdBufBeginInfo),
            "Failed to begin command buffer.");

        auto followUp = func(*cmd);

        VKCHECK(vkEndCommandBuffer(cmd->vkCmdBuf),
            "Failed to end command buffer.");

        // submit command buffer
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence vkFence;
        VKCHECK(vkCreateFence(vkDevice, &fenceCreateInfo, VK_NULL_HANDLE, &vkFence),
            "Failed to create fence.");

        // handle vkFence lifetime
        auto fencePtr = ke::VulkanFence::create(vkDevice, vkFence);

        VkCommandBufferSubmitInfo cmdBufSubmitInfo{};
        cmdBufSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdBufSubmitInfo.commandBuffer = cmd->vkCmdBuf;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdBufSubmitInfo;

        VKCHECK(queue.submit(1, &submitInfo, vkFence),
            "Failed to submit command buffer.");

        if (!awaitFence) {
            std::lock_guard<std::mutex> lock(waitingFenceMtx);
            // function lambda executes a copy, and we cant copy a unique ptr...
            std::shared_ptr<CommandBuffer> sCmd = std::move(cmd);
            vkFenceActions[std::move(fencePtr)] = [sCmd, followUp]() mutable {
                // cmd.dispose(); // smart ptr takes care of this for us
                if (followUp)
                    followUp();
            };

            return;
        }

        vkWaitForFences(vkDevice, 1, &vkFence, true, LLONG_MAX);

        if (followUp)
            followUp();
    }

    SamplerCache& VulkanContext::getSamplerCache() {
        return *samplerCache;
    }

    DescriptorSetLayoutCache& VulkanContext::getDescSetLayoutCache() {
        return *descSetLayoutCache;
    }

    RenderPass& VulkanContext::getRenderPass(int i) {
        return *renderPasses[i];
    }

    void FrameSyncObjects::createFences(VkDevice device, std::unique_ptr<ke::VulkanFence>* fences) {
        for (auto i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkFence newFence;
            VKCHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &newFence),
                "Failed to create fence");

            fences[i] = ke::VulkanFence::create(device, newFence);
        }
    }

    void FrameSyncObjects::createSemaphores(VkDevice device, std::unique_ptr<ke::VulkanSemaphore>* semaphores) {
        for (auto i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VkSemaphore newSemaphore;
            VKCHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &newSemaphore),
                "Failed to create semaphore");

            semaphores[i] = ke::VulkanSemaphore::create(device, newSemaphore);
        }
    }

    void FrameSyncObjects::init() {
        createFences(vkDevice, frameFences);
        createSemaphores(vkDevice, frameSemaphores);
        createSemaphores(vkDevice, imageAcquireSemaphores);
    }
} // namespace ke