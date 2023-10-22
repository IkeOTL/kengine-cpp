#include "VulkanContext.hpp"
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <algorithm>

VulkanContext::VulkanContext()
{
}

VulkanContext::~VulkanContext()
{
    if (vkInstance != VK_NULL_HANDLE) {
        auto funcDestroyDebug = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
        if (funcDestroyDebug)
            funcDestroyDebug(vkInstance, debugCallbackHandle, nullptr);

        //keeps crashing look into it later
        vkDestroyInstance(vkInstance, nullptr);
    }
}

void VulkanContext::init(Window& window, bool validationOn) {
    createVkInstance(validationOn);

    if (validationOn)
        setupDebugging();

    window.createSurface(vkInstance, surface);
    grabFirstPhysicalDevice();

    colorFormatAndSpace.init(physicalDevice, surface);
}

void VulkanContext::createVkInstance(bool validationOn)
{
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
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME
    };
    const size_t myExtCnt = sizeof(myExts) / sizeof(myExts[0]);

    std::vector<const char*> allExts(glfwExts, glfwExts + glfwExtCnt);
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

    std::vector<VkLayerProperties> availableLayers(layerCount);

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


void VulkanContext::setupDebugging()
{
    auto flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

    VkDebugReportCallbackCreateInfoEXT vkDebugCbCreateInfo{};
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
        physicalDevice = device;
        break;
    }

    // load up gpu details
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);
}