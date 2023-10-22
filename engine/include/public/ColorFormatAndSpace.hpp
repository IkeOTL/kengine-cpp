#pragma once
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>

class ColorFormatAndSpace {
private:
    VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

public:
    void init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    VkFormat getColorFormat() const {
        return colorFormat;
    }

    VkColorSpaceKHR getColorSpace() const {
        return colorSpace;
    }

    VkFormat getDepthFormat() const {
        return depthFormat;
    }

};