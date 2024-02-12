#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <kengine/vulkan/DrawObjectBuffer.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>

DescriptorSetLayoutConfig compositionLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 2, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 3, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 4, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 5, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 6, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 7, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT },
    DescriptorSetLayoutBindingConfig{ 8, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT }
};

void DeferredCompositionPbrPipeline::bind(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, getVkPipeline());

    auto descSet = descSetAllocator.getGlobalDescriptorSet("deferred-composition", DeferredCompositionPbrPipeline::compositionLayout);

    // TODO: check alignments
    uint32_t dynamicOffsets[] = {
        frameIndex * LightsManager.alignedFrameSize(vkCxt),
        frameIndex * ShadowCascadeData::alignedFrameSize(vkCxt),
        frameIndex * MaterialsBuffer::alignedFrameSize(vkCxt)
    };

    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        getVkPipelineLayout(),
        0,
        1, &descSet,
        3, dynamicOffsets
    );
}

void DeferredCompositionPbrPipeline::loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) {
    dst.push_back(DeferredCompositionPbrPipeline::compositionLayout);
}

VkPipelineLayout DeferredCompositionPbrPipeline::createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) {


    return pipelineLayout;
}

VkPipeline DeferredCompositionPbrPipeline::createPipeline(VkDevice device, RenderPass& renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) {

    return newPipeline;
}