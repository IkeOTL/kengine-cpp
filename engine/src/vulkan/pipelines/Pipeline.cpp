#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <fstream>

void Pipeline::init(VulkanContext& vkContext, RenderPass& renderPass, DescriptorSetLayoutCache& layoutCache, glm::uvec2 extents) {
    loadDescriptorSetLayoutConfigs(descSetLayoutConfigs);
    vkPipelineLayout = createPipelineLayout(vkContext, layoutCache);
    vkPipeline = createPipeline(vkContext.getVkDevice(), renderPass, vkPipelineLayout, extents);
}

void Pipeline::createVertexInputDescriptions(
    const VertexFormatDescriptor& descriptor,
    VkVertexInputBindingDescription& bindingDescription,
    std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
{
    bindingDescription.binding = 0;
    bindingDescription.stride = descriptor.stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions.clear();
    for (const auto& attr : descriptor.attributes) {
        VkVertexInputAttributeDescription attributeDescription{};
        attributeDescription.binding = 0;
        attributeDescription.location = attr.location;
        attributeDescription.format = attr.format;
        attributeDescription.offset = attr.offset;
        attributeDescriptions.push_back(attributeDescription);
    }
}

void Pipeline::loadShader(VkDevice device, std::string filePath, VkShaderStageFlagBits stage, std::vector<VkPipelineShaderStageCreateInfo>& dest) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader file");

    auto fileSize = (size_t)file.tellg();
    std::vector<char> shaderCode(fileSize);

    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();

    // Create the shader module
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = shaderCode.size();
    moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    VKCHECK(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule),
        "Failed to create shader module");

    // Create the shader stage info
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = stage;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";

    dest.push_back(shaderStageInfo);
}