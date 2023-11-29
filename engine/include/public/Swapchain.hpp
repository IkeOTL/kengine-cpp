#pragma once
#include <VulkanInclude.hpp>
#include <vector>
#include <GpuImage.hpp>
#include <glm/vec2.hpp>
#include <ColorFormatAndSpace.hpp>

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