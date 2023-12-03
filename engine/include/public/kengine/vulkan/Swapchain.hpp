#pragma once
#include <kengine/vulkan/GpuImage.hpp>
#include <kengine/vulkan/ColorFormatAndSpace.hpp>
#include <kengine/vulkan/VulkanInclude.hpp>

#include <vector>
#include <glm/vec2.hpp>

class Swapchain {
public:
    Swapchain(VkDevice vkDevice)
        : vkDevice(vkDevice) {}

    std::unique_ptr<Swapchain> replace(VkPhysicalDevice physicalDevice, VkDevice device,
        int newWidth, int newHeight, VkSurfaceKHR surface, ColorFormatAndSpace& colorFormatAndSpace);

private:
    const VkDevice vkDevice;
    VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> vkImages{};
    std::vector<VkImageView> vkImageViews{};
    glm::uvec2 imageExtents{};

};