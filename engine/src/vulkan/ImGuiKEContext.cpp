#include <kengine/vulkan/ImGuiKEContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/Window.hpp>
#include <array>

ImGuiKEContext::~ImGuiKEContext() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

VkDescriptorPool ImGuiKEContext::createDescriptorPool() {
    // Typically ImGui will need at least the following descriptor types
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount = 500;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 500;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Optional: Allows individual descriptor sets to be freed
    poolInfo.maxSets = 1000; // Maximum number of descriptor sets that can be allocated from the pool
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool descriptorPool;
    VKCHECK(vkCreateDescriptorPool(vkCtx.getVkDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS,
        "failed to create descriptor pool!");

    descPool = descriptorPool;
    return descPool;
}

void ImGuiKEContext::init(Window& window) {
    ImGui::CreateContext();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkCtx.getVkInstance();
    init_info.PhysicalDevice = vkCtx.getVkPhysicalDevice();
    init_info.Device = vkCtx.getVkDevice();
    init_info.QueueFamily = vkCtx.getGfxQueueFamilyIndex();
    init_info.Queue = vkCtx.getGraphicsQueue().getVkQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = createDescriptorPool();
    init_info.MinImageCount = 2;
    init_info.ImageCount = VulkanContext::FRAME_OVERLAP;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.RenderPass = vkCtx.getRenderPass(0).getVkRenderPass();
    init_info.Subpass = 3;
    //init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiKEContext::draw(RenderFrameContext& rCtx) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(&showDemoWindow);

    draw();

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, rCtx.cmd);
}
