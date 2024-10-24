#pragma once

#include "VulkanObject.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

namespace ke {
    class VulkanContext;
    class Window;
    class RenderFrameContext;

    class ImGuiKEContext {
    private:
        VulkanContext& vkCtx;
        std::unique_ptr<ke::VulkanDescriptorPool> descPool = nullptr;
        bool isVisible = true;

        VkDescriptorPool createDescriptorPool();

    protected:
        virtual void draw() = 0;

    public:
        ImGuiKEContext(VulkanContext& vkCtx) : vkCtx(vkCtx) {}
        ~ImGuiKEContext();

        void init(Window& window);
        void draw(RenderFrameContext& rCtx);
    };
} // namespace ke