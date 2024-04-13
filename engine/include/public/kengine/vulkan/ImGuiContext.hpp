#pragma once

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

class VulkanContext;
class Window;

class ImGuiContext {
private:
    VulkanContext& vkCtx;
    VkDescriptorPool descPool;

    VkDescriptorPool createDescriptorPool();

public:
    ImGuiContext(VulkanContext& vkCtx) : vkCtx(vkCtx) {}

    void init(Window& window);
};