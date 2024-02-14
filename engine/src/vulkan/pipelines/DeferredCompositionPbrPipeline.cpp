#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <kengine/vulkan/DrawObjectBuffer.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/LightsManager.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/ShadowCascade.hpp>

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

void DeferredCompositionPbrPipeline::bind(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, getVkPipeline());

    auto descSet = descSetAllocator.getGlobalDescriptorSet("deferred-composition", DeferredCompositionPbrPipeline::compositionLayout);

    // TODO: check alignments
    uint32_t dynamicOffsets[] = {
        frameIndex * LightsManager::alignedFrameSize(vkCxt),
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
    std::vector<VkDescriptorSetLayout> setLayouts;
    for (const auto& config : descSetLayoutConfigs)
        setLayouts.push_back(layoutCache.getLayout(config));

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VkPipelineLayout pipelineLayout;
    VKCHECK(vkCreatePipelineLayout(vkContext.getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout),
        "Failed to create pipeline layout");

    return pipelineLayout;
}

VkPipeline DeferredCompositionPbrPipeline::createPipeline(VkDevice device, RenderPass& renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo;
    loadShader(device, "res/src/deferred/pbr/composition.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, shaderStagesCreateInfo);
    loadShader(device, "res/src/deferred/pbr/composition.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, shaderStagesCreateInfo);

    // specialization
    {
        VkSpecializationMapEntry specializationEntry{};
        specializationEntry.constantID = 0;
        specializationEntry.offset = 0;
        specializationEntry.size = sizeof(int32_t);

        VkSpecializationInfo specializationInfo{};
        specializationInfo.mapEntryCount = 1;
        specializationInfo.pMapEntries = &specializationEntry;
        specializationInfo.dataSize = sizeof(LightsManager::MAX_LIGHTS);
        specializationInfo.pData = &LightsManager::MAX_LIGHTS;

        shaderStagesCreateInfo[1].pSpecializationInfo = &specializationInfo;
    }

    VkPipelineDynamicStateCreateInfo dynamicState{};
    {
        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
    }

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.front = depthStencilState.back;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStagesCreateInfo.size());
    pipelineInfo.pStages = shaderStagesCreateInfo.data();
    pipelineInfo.pVertexInputState = &vi;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.renderPass = renderPass.getVkRenderPass();
    pipelineInfo.subpass = 1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline newPipeline;
    VKCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline),
        "Failed to create pipeline");

    return newPipeline;
}