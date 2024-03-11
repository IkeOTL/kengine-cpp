#include <kengine/vulkan/Swapchain.hpp>

std::unique_ptr<Swapchain> Swapchain::replace(VkPhysicalDevice physicalDevice, VkDevice device,
    int newWidth, int newHeight, VkSurfaceKHR surface, ColorFormatAndSpace& colorFormatAndSpace) {

    auto presentModeCount = 0u;
    VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, VK_NULL_HANDLE),
        "Failed to get number of physical device surface presentation modes");

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()),
        "Failed to get physical device surface presentation modes");

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    VKCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities),
        "Failed to get physical device surface capabilities");

    auto swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    //            int swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    //            for (int i = 0; i < presentModeCount; i++) {
    //                if (pPresentModes.get(i) == VK_PRESENT_MODE_MAILBOX_KHR) {
    //                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    //                    break;
    //                }
    //
    //                if (swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR && pPresentModes.get(i) == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //                    swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    //                }
    //            }

    auto desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount)
        desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;

    auto currentExtent = surfaceCapabilities.currentExtent;
    imageExtents = glm::uvec2(currentExtent.width, currentExtent.height);
    if (imageExtents.x == -1 || imageExtents.y == -1)
        imageExtents = { newWidth, newHeight };

    auto preTransform = surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        : surfaceCapabilities.currentTransform;

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface;
    sci.minImageCount = desiredNumberOfSwapchainImages;
    sci.imageFormat = colorFormatAndSpace.getColorFormat();
    sci.imageColorSpace = colorFormatAndSpace.getColorSpace();
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.preTransform = preTransform;
    sci.imageArrayLayers = 1;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.presentMode = swapchainPresentMode;
    sci.oldSwapchain = vkSwapchain;
    sci.clipped = true;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.imageExtent = { imageExtents.x, imageExtents.y };

    VkSwapchainKHR newVkSwapchain;
    VKCHECK(vkCreateSwapchainKHR(device, &sci, VK_NULL_HANDLE, &newVkSwapchain),
        "Failed create swapchain.");

    if (vkSwapchain != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);

        //destroy image views then swapchain, ensure we destory all things in the right order
        for (auto iv : vkImageViews)
            vkDestroyImageView(device, iv, VK_NULL_HANDLE);

        vkDestroySwapchainKHR(device, vkSwapchain, VK_NULL_HANDLE);

        // set null ptr so the destructor can early exit in case of already disposed
        vkSwapchain = VK_NULL_HANDLE;
    }

    auto imageCount = 0u;
    VKCHECK(vkGetSwapchainImagesKHR(device, newVkSwapchain, &imageCount, VK_NULL_HANDLE),
        "Failed to get number of swapchain images");

    std::vector<VkImage> swapchainImages(imageCount);
    VKCHECK(vkGetSwapchainImagesKHR(device, newVkSwapchain, &imageCount, swapchainImages.data()),
        "Failed to get swapchain images");

    // create new swapchain
    {
        auto swapchain = std::make_unique<Swapchain>(device);
        swapchain->vkSwapchain = newVkSwapchain;

        VkImageViewCreateInfo ivci{};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.format = colorFormatAndSpace.getColorFormat();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.layerCount = 1;

        swapchain->vkImages.resize(imageCount);
        swapchain->vkImageViews.resize(imageCount);
        for (auto i = 0; i < imageCount; i++) {
            swapchain->vkImages[i] = swapchainImages[i];

            ivci.image = swapchainImages[i];
            VKCHECK(vkCreateImageView(device, &ivci, VK_NULL_HANDLE, &swapchain->vkImageViews[i]),
                "Failed to create image view");
        }

        swapchain->imageExtents = this->imageExtents;

        return swapchain;
    }
}