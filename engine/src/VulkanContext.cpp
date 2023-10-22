#include "VulkanContext.hpp"
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

VulkanContext::VulkanContext()
{
}

VulkanContext::~VulkanContext()
{
    if (vkInstance != VK_NULL_HANDLE)
        vkDestroyInstance(vkInstance, nullptr);
}

std::vector<const char*> getVkExtensions() {
    auto glfwExtCnt = 0u;
    auto glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCnt);

    const char* myExts[] = {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };
    const size_t myExtCnt = sizeof(myExts) / sizeof(myExts[0]);

    std::vector<const char*>  allExts(glfwExts, glfwExts + glfwExtCnt);
    allExts.insert(allExts.end(), myExts, myExts + myExtCnt);

    return allExts;
}

void VulkanContext::init(Window& window, bool validationOn) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Demo";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "kengine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    auto allExtensions = getVkExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(allExtensions.size());
    createInfo.ppEnabledExtensionNames = allExtensions.data();

    //if (validationOn) {
    //    createInfo.ppEnabledLayerNames
    //}

    auto result = vkCreateInstance(&createInfo, nullptr, &vkInstance);

    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance.");
}