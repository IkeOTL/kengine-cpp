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
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount = 500;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 500;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool descriptorPool;
    VKCHECK(vkCreateDescriptorPool(vkCtx.getVkDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS,
        "failed to create descriptor pool!");

    descPool = std::make_unique<ke::VulkanDescriptorPool>(vkCtx.getVkDevice(), descriptorPool);

    return descriptorPool;
}

void ImGuiKEContext::init(Window& window) {
    ImGui::CreateContext();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info{};
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

#ifdef KE_DEBUG_RENDER
    init_info.Subpass = 4;
#else
    init_info.Subpass = 3;
#endif

    //init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // to allow imgui to stop event propagation
    window.setImGuiIO(&ImGui::GetIO());
}

void ImGuiKEContext::draw(RenderFrameContext& rCtx) {
    if (!isVisible)
        return;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow(&showDemoWindow);

    draw();

    ImGui::Render();
    auto* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, rCtx.cmd);
}
