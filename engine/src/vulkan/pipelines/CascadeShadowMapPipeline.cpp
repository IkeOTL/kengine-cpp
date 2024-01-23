#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>

DescriptorSetLayoutConfig shadowPassLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT },
    DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
};

DescriptorSetLayoutConfig cascadeViewProjLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
};

DescriptorSetLayoutConfig textureLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
};

void CascadeShadowMapPipeline::loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) {
    dst.push_back(cascadeViewProjLayout);
    dst.push_back(shadowPassLayout);
    dst.push_back(textureLayout);
}

VkPipelineLayout CascadeShadowMapPipeline::createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) {
    std::vector<VkDescriptorSetLayout> setLayouts;
    for (const auto& config : descSetLayoutConfigs) 
        setLayouts.push_back(layoutCache.getLayout(config));

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int32_t);

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

VkPipeline CascadeShadowMapPipeline::createPipeline(VkDevice device, RenderPass& renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) {
    return VkPipeline();
}

void CascadeShadowMapPipeline::bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) {
}
