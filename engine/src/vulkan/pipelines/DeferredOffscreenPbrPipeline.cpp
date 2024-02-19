#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <kengine/vulkan/DrawObjectBuffer.hpp>
#include <kengine/vulkan/material/PbrMaterialConfig.hpp>
#include <kengine/vulkan/Camera.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/SceneData.hpp>

void DeferredOffscreenPbrPipeline::bind(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, getVkPipeline());

    VkDescriptorSet descriptorSets[] = {
       descSetAllocator.getGlobalDescriptorSet("deferred-global-layout", PipelineCache::globalLayout),
       descSetAllocator.getGlobalDescriptorSet("deferred-gbuffer", DeferredOffscreenPbrPipeline::objectLayout)
    };

    // TODO: check alignments
    uint32_t dynamicOffsets[] = {
        frameIndex * SceneData::alignedFrameSize(vkCxt),
        frameIndex * DrawObjectBuffer::alignedFrameSize(vkCxt),
        frameIndex * RenderContext::MAX_INSTANCES * sizeof(uint32_t),
        frameIndex * MaterialsBuffer::alignedFrameSize(vkCxt)
    };

    // Single vkCmdBindDescriptorSets call
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        getVkPipelineLayout(),
        0,
        2, descriptorSets,
        4, dynamicOffsets
    );
}

void DeferredOffscreenPbrPipeline::loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) {
    dst.push_back(PipelineCache::globalLayout);
    dst.push_back(objectLayout);
    dst.push_back(pbrTextureLayout);
}

VkPipelineLayout DeferredOffscreenPbrPipeline::createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) {
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

VkPipeline DeferredOffscreenPbrPipeline::createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo;
    loadShader(device, "res/src/gbuffer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, shaderStagesCreateInfo);
    loadShader(device, "res/src/gbuffer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, shaderStagesCreateInfo);

    // Specialization map entries
    std::array<VkSpecializationMapEntry, 2> specializationEntries{};
    specializationEntries[0].constantID = 0;
    specializationEntries[0].offset = 0;
    specializationEntries[0].size = sizeof(float);
    specializationEntries[1].constantID = 1;
    specializationEntries[1].offset = sizeof(float);
    specializationEntries[1].size = sizeof(float);

    // Specialization data
    float specializationData[] = { Camera::NEAR_CLIP, Camera::FAR_CLIP };

    VkSpecializationInfo specializationInfo{};
    specializationInfo.mapEntryCount = static_cast<uint32_t>(specializationEntries.size());
    specializationInfo.pMapEntries = specializationEntries.data();
    specializationInfo.dataSize = sizeof(specializationData);
    specializationInfo.pData = specializationData;

    // Assuming shaderStages is a previously prepared array of VkPipelineShaderStageCreateInfo
    shaderStagesCreateInfo[1].pSpecializationInfo = &specializationInfo;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // vertex input info
    auto texturedVertexFormat = VertexFormatDescriptor{
        sizeof(TexturedVertex),
        {
            {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(TexturedVertex, position)},
            {1, VK_FORMAT_R32G32_SFLOAT, offsetof(TexturedVertex, texCoords)},
            {2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(TexturedVertex, normal)},
            {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(TexturedVertex, tangent)}
        }
    };

    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    createVertexInputDescriptions(texturedVertexFormat, bindingDescription, attributeDescriptions);

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &bindingDescription;
    vi.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vi.pVertexAttributeDescriptions = attributeDescriptions.data();
    //

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    // Multisample state
    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color blend attachment states
    std::array<VkPipelineColorBlendAttachmentState, 6> colorBlendAttachments{};
    for (auto& attachment : colorBlendAttachments) {
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        attachment.blendEnable = VK_FALSE;
    }

    // Color blending state
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();

    // Depth stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.front = depthStencilState.back;

    // Dynamic state
    std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stageCount = shaderStagesCreateInfo.size();
    pipelineInfo.pStages = shaderStagesCreateInfo.data();
    pipelineInfo.pVertexInputState = &vi;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.renderPass = renderPass->getVkRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline newPipeline;
    VKCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline),
        "Failed to create pipeline");

    return newPipeline;
}