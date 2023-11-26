#pragma once
#include <VulkanInclude.hpp>
#include <vector>
#include <GpuImage.hpp>
#include <glm/vec2.hpp>

class Swapchain {
public:
    Swapchain(VkDevice vkDevice)
        : vkDevice(vkDevice) {}

    ~Swapchain();

private:
    const VkDevice vkDevice;
    std::vector<std::unique_ptr<GpuImageView>> imageViews;
    glm::uvec2 imageExtents;
};