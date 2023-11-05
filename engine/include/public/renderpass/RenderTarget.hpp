#pragma once
#include <vulkan/vulkan.h>
#include "RenderPass.hpp"

class RenderTarget {
private:
    RenderPass<RenderTarget>& renderPass;
    VkFramebuffer vkFrameBuffer = VK_NULL_HANDLE;

public:

};