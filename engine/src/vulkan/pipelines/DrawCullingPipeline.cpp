#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/DrawObjectBuffer.hpp>

DescriptorSetLayoutConfig cullingLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
    DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
    DescriptorSetLayoutBindingConfig{ 2, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
    DescriptorSetLayoutBindingConfig{ 3, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT }
};

void DrawCullingPipeline::bind(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, getVkPipeline());

    auto set0 = descSetAllocator.getGlobalDescriptorSet("deferred-culling", cullingLayout);

    // TODO: check alignments
    uint32_t dynamicOffsets[] = {
        frameIndex * RenderContext::MAX_INSTANCES * sizeof(VkDrawIndexedIndirectCommand),
        frameIndex * RenderContext::MAX_INSTANCES * (2 * sizeof(uint32_t)),
        frameIndex * DrawObjectBuffer::alignedFrameSize(vkCxt),
        frameIndex * RenderContext::MAX_INSTANCES * sizeof(uint32_t)
    };

    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        getVkPipelineLayout(),
        0,
        1, &set0,
        4, dynamicOffsets
    );
}


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
    pushConstantRange.size = sizeof(DrawCullingPipeline::PushConstant);

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
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo;
    loadShader(device, "res/src/deferred/object-cull.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT, shaderStagesCreateInfo);

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stage = shaderStagesCreateInfo[0];
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1; // Optional, but good practice

    VkPipeline newPipeline;
    VKCHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline),
        "Failed to create pipeline");

    return newPipeline;
}