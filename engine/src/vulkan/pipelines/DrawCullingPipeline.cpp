#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>

DescriptorSetLayoutConfig cullingLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
    DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
    DescriptorSetLayoutBindingConfig{ 2, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
    DescriptorSetLayoutBindingConfig{ 3, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT }
};

void DrawCullingPipeline::loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) {
    dst.push_back(cullingLayout);
}

VkPipelineLayout DrawCullingPipeline::createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) {
    std::vector<VkDescriptorSetLayout> setLayouts;
    for (const auto& config : descSetLayoutConfigs) 
        setLayouts.push_back(layoutCache.getLayout(config));

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = (16 * sizeof(float)) + (4 * sizeof(float)) + (4 * sizeof(float)) + sizeof(int);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VkPipelineLayout pipelineLayout;
    VKCHECK(vkCreatePipelineLayout(vkContext.getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
        "Failed to create pipeline layout");

    return pipelineLayout;
}

VkPipeline DrawCullingPipeline::createPipeline(VkDevice device, RenderPass& renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) {
    return VkPipeline();
}

void DrawCullingPipeline::bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) {
}
