#include "ColorFormatAndSpace.hpp"

void ColorFormatAndSpace::init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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

    colorFormat = chosenFormat.format;
    colorSpace = chosenFormat.colorSpace;
}
