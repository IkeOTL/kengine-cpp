#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <stdexcept>
#include <vector>

namespace ke {
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
} // namespace ke