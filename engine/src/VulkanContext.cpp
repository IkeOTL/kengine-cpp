#define VMA_IMPLEMENTATION
#include "VulkanContext.hpp"
#include "renderpass/RenderPass.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

VulkanContext::VulkanContext() {}

VulkanContext::~VulkanContext() {
    if (vkInstance != VK_NULL_HANDLE) {
        auto funcDestroyDebug = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
        if (funcDestroyDebug)
            funcDestroyDebug(vkInstance, debugCallbackHandle, nullptr);

        vmaDestroyAllocator(vmaAllocator);

        vkDestroyInstance(vkInstance, nullptr);
    }
}

void VulkanContext::init(Window& window, bool validationOn) {
    createVkInstance(validationOn);

    if (validationOn)
        setupDebugging();

    window.createSurface(vkInstance, vkSurface);
    grabFirstPhysicalDevice();
    colorFormatAndSpace.init(vkPhysicalDevice, vkSurface);

    queueFamilies.init(vkPhysicalDevice);
    gfxQueueFamilyIndex = queueFamilies.getGfxCompXferQueues()[0];
    compQueueFamilyIndex = gfxQueueFamilyIndex;
    xferQueueFamilyIndex = queueFamilies.getTransferQueues()[0];

    createDevice();
    createQueues();
    createVmaAllocator();
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
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    };
    const auto myExtCnt = sizeof(myExts) / sizeof(myExts[0]);

    auto allExts = std::vector<const char*>(glfwExts, glfwExts + glfwExtCnt);
    allExts.insert(allExts.end(), myExts, myExts + myExtCnt);

    if (allExts.size()) {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(allExts.size());
        createInfo.ppEnabledExtensionNames = allExts.data();
    }

    // might change this logic since we might need layers that arent validation layers
    if (!validationOn) {
        auto result = vkCreateInstance(&createInfo, nullptr, &vkInstance);

        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan instance.");

        return;
    }

    // apply validation layers
    auto layerCount = 0u;
    auto lPropsResult = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (lPropsResult != VK_SUCCESS)
        throw std::runtime_error("Failed to get the number of instance layers.");

    auto availableLayers = std::vector<VkLayerProperties>(layerCount);

    lPropsResult = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    if (lPropsResult != VK_SUCCESS)
        throw std::runtime_error("Failed to get instance layers properties.");

    std::vector<const char*> desiredLayers = {
          "VK_LAYER_LUNARG_standard_validation",
          "VK_LAYER_KHRONOS_validation"
    };

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

    for (const auto& layerProperties : layersToEnable) {
        std::cout << "Enabled layer: " << layerProperties;
    }

    if (layersToEnable.size()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(layersToEnable.size());
        createInfo.ppEnabledLayerNames = layersToEnable.data();
    }

    auto result = vkCreateInstance(&createInfo, nullptr, &vkInstance);

    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance.");
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
        void* userData) -> VkBool32
    {
        std::cerr << "Validation Layer: " << msg << std::endl;
        return VK_FALSE;
    };

    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

    if (!func)
        throw std::runtime_error("Could not get address of vkCreateDebugReportCallbackEXT.");

    VkDebugReportCallbackEXT debugCallbackHandle;
    if (func(vkInstance, &vkDebugCbCreateInfo, nullptr, &debugCallbackHandle) != VK_SUCCESS)
        throw std::runtime_error("Failed to set up debug callback.");
}

void VulkanContext::grabFirstPhysicalDevice() {
    auto deviceCnt = 0u;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCnt, VK_NULL_HANDLE);

    if (!deviceCnt)
        throw std::runtime_error("No devices available.");

    std::vector<VkPhysicalDevice> availableDevices(deviceCnt);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCnt, availableDevices.data());

    for (const auto& device : availableDevices) {
        if (!true)  //eventually need to check if the device is suitable
            continue;

        // for now simply return the first one we see
        vkPhysicalDevice = device;
        break;
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

    auto sync2Features = VkPhysicalDeviceSynchronization2FeaturesKHR{};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    sync2Features.synchronization2 = true;

    VkDeviceQueueCreateInfo queueCreateInfos[2] = { graphicsQueueCreateInfo, transferQueueCreateInfo };

    std::vector<const char*> desiredLayers = {
         VK_KHR_SWAPCHAIN_EXTENSION_NAME,
         VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
    };

    auto createDeviceInfo = VkDeviceCreateInfo{};
    createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createDeviceInfo.pNext = &sync2Features;
    createDeviceInfo.queueCreateInfoCount = 2;
    createDeviceInfo.pQueueCreateInfos = queueCreateInfos;
    createDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(desiredLayers.size());
    createDeviceInfo.ppEnabledExtensionNames = desiredLayers.data();

    vkCreateDevice(vkPhysicalDevice, &createDeviceInfo, nullptr, &vkDevice);
}

void VulkanContext::createQueues() {
    graphicsQueue = std::make_shared<VulkanQueue>(vkDevice, gfxQueueFamilyIndex);
    computeQueue = graphicsQueue;
    transferQueue = std::make_unique<VulkanQueue>(vkDevice, xferQueueFamilyIndex);

    graphicsQueue->init();
    transferQueue->init();
}

void VulkanContext::createVmaAllocator() {
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
    
    //optional
    vmaVkFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties2");
    
    vmaVkFunctions.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(vkDevice, "vkGetBufferMemoryRequirements2");
    vmaVkFunctions.vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(vkDevice, "vkGetImageMemoryRequirements2");
    vmaVkFunctions.vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(vkDevice, "vkBindBufferMemory2");
    vmaVkFunctions.vkBindImageMemory2KHR = (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(vkDevice, "vkBindImageMemory2");
    vmaVkFunctions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetDeviceBufferMemoryRequirements");
    vmaVkFunctions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetDeviceImageMemoryRequirements");

    allocatorInfo.pVulkanFunctions = &vmaVkFunctions;

    auto result = vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to initialize VMA allocator.");

}