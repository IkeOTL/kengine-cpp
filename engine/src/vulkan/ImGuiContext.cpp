#include <kengine/vulkan/ImGuiContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/Window.hpp>

void ImGuiContext::init(Window& window)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkCtx.getVkInstance();
    init_info.PhysicalDevice = vkCtx.getVkPhysicalDevice();
    init_info.Device = vkCtx.getVkDevice();
    init_info.QueueFamily = vkCtx.getGfxQueueFamilyIndex();
    init_info.Queue = vkCtx.getGraphicsQueue().getVkQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool; // You need to create this descriptor pool
    init_info.MinImageCount = minImageCount;
    init_info.ImageCount = imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    // Load Fonts
    io.Fonts->AddFontDefault();

    // Upload Fonts
    {
        VkCommandBuffer command_buffer = beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        endSingleTimeCommands(command_buffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}
