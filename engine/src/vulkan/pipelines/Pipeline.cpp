#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>
#include <kengine/EngineConfig.hpp>

#include <fstream>
#include <filesystem>

namespace ke {
    Pipeline::~Pipeline() {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
        vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
    }

    const DescriptorSetLayoutConfig& Pipeline::getDescSetLayoutConfig(int i) const {
        return descSetLayoutConfigs[i];
    }

    void Pipeline::init(VulkanContext& vkContext, RenderPass* renderPass, DescriptorSetLayoutCache& layoutCache, glm::uvec2 extents) {
        loadDescriptorSetLayoutConfigs(descSetLayoutConfigs);
        vkPipelineLayout = createPipelineLayout(vkContext, layoutCache);
        vkPipeline = createPipeline(vkContext.getVkDevice(), renderPass, vkPipelineLayout, extents);
    }

    void Pipeline::createVertexInputDescriptions(
        const VertexFormatDescriptor& descriptor,
        VkVertexInputBindingDescription& bindingDescription,
        std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) {
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

    // need to move shader loading to a asset cache
    void Pipeline::loadShader(std::string shaderFile, VkShaderStageFlagBits stage, std::vector<VkPipelineShaderStageCreateInfo>& dest) {
        auto assetPath = std::filesystem::path(EngineConfig::getInstance().getAssetRoot()) / shaderFile;

        std::ifstream file(assetPath.string(), std::ios::ate | std::ios::binary);
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
        VKCHECK(vkCreateShaderModule(vkDevice, &moduleCreateInfo, nullptr, &shaderModule),
            "Failed to create shader module");

        shaderModules.push_back(std::make_unique<ke::VulkanShaderModule>(vkDevice, shaderModule));

        // Create the shader stage info
        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = stage;
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = "main";

        dest.push_back(shaderStageInfo);
    }
} // namespace ke