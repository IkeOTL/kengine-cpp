#pragma once

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

class VulkanContext;
class Window;
class RenderFrameContext;

class ImGuiKEContext {
private:
    VulkanContext& vkCtx;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    bool isVisible = true;

    VkDescriptorPool createDescriptorPool();

protected:
    virtual void draw() = 0;

public:
    ImGuiKEContext(VulkanContext& vkCtx) : vkCtx(vkCtx) {}
    ~ImGuiKEContext();

    void init(Window& window, bool isDebugRendering);
    void draw(RenderFrameContext& rCtx);
};