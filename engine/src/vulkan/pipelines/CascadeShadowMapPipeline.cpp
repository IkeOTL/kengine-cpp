#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <kengine/vulkan/RenderContext.hpp>
#include <kengine/vulkan/DrawObjectBuffer.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/ShadowCascade.hpp>

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


void CascadeShadowMapPipeline::bind(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, getVkPipeline());

    VkDescriptorSet descriptorSets[] = {
        descSetAllocator.getGlobalDescriptorSet("shadow-pass0", shadowPassLayout),
        descSetAllocator.getGlobalDescriptorSet("cascade", cascadeViewProjLayout)
    };

    // TODO: check alignments
    uint32_t dynamicOffsets[] = {
        frameIndex * ShadowCascadeData::SHADOW_CASCADE_COUNT * 16 * sizeof(float),
        frameIndex * DrawObjectBuffer::alignedFrameSize(vkCxt),
        frameIndex * RenderContext::MAX_INSTANCES * sizeof(int)
    };

    // Single vkCmdBindDescriptorSets call
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        getVkPipelineLayout(),
        0,
        2, descriptorSets,
        3, dynamicOffsets
    );
}

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
    pushConstantRange.size = sizeof(CascadeShadowMapPipeline::PushConstant);

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
    std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfo;
    loadShader(device, "res/src/deferred/cascade-shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, shaderStagesCreateInfo);
    loadShader(device, "res/src/deferred/cascade-shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, shaderStagesCreateInfo);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extents.x);
    viewport.height = static_cast<float>(extents.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { extents.x, extents.y };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

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

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;

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
    pipelineInfo.stageCount = shaderStagesCreateInfo.size();
    pipelineInfo.pStages = shaderStagesCreateInfo.data();
    pipelineInfo.pVertexInputState = &vi;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.renderPass = renderPass.getVkRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline newPipeline;
    VKCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline),
        "Failed to create pipeline");

    return newPipeline;
}
