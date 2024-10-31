#include <kengine/vulkan/ColorFormatAndSpace.hpp>

namespace ke {
    void ColorFormatAndSpace::init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        // surface colorformat
        {
            VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
            surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
            surfaceInfo.surface = surface;

            auto surfaceFormatCnt = 0u;
            vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo, &surfaceFormatCnt, VK_NULL_HANDLE);

            if (!surfaceFormatCnt)
                throw std::runtime_error("No surface formats available.");

            std::vector<VkSurfaceFormat2KHR> availableSurfaceFormats(surfaceFormatCnt);
            for (auto& format : availableSurfaceFormats)
                format.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR; // is this even needed?

            vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo, &surfaceFormatCnt, availableSurfaceFormats.data());

            VkSurfaceFormatKHR chosenFormat{};
            for (const auto& availableFormat : availableSurfaceFormats) {
                if (availableFormat.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                    && availableFormat.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { // hmmmmmm
                    chosenFormat = availableFormat.surfaceFormat;
                    break;
                }
            }

            this->colorFormat = chosenFormat.format;
            this->colorSpace = chosenFormat.colorSpace;
        }

        // device depthformat
        {
            // Since all depth formats may be optional, we need to find a suitable depth format to use
            // Start with the highest precision packed format
            std::vector<VkFormat> depthFormats = {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM};

            for (VkFormat format : depthFormats) {
                VkFormatProperties formatProps;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
                // Format must support depth stencil attachment for optimal tiling
                if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                    this->depthFormat = format;
                    break;
                }
            }
        }
    }
} // namespace ke
