#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>
#include <vector>
#include <glm/vec2.hpp>

class VulkanContext;
class RenderPass;
class DescriptorSetAllocator;
class DescriptorSetLayoutCache;

struct VertexFormatAttribute {
    uint32_t location;
    VkFormat format;
    size_t offset;
};

struct VertexFormatDescriptor {
    size_t stride;
    std::vector<VertexFormatAttribute> attributes;
};

class Pipeline {
private:
    VkPipeline vkPipeline;
    VkPipelineLayout vkPipelineLayout;

protected:
    std::vector<DescriptorSetLayoutConfig> descSetLayoutConfigs{};

    virtual void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) = 0;

    static void createVertexInputDescriptions(
        const VertexFormatDescriptor& descriptor,
        VkVertexInputBindingDescription& bindingDescription,
        std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

    static void loadShader(VkDevice device, std::string filePath, VkShaderStageFlagBits stage, std::vector<VkPipelineShaderStageCreateInfo>& dest);

public:
    virtual ~Pipeline() = default;

    VkPipeline getVkPipeline() {
        return vkPipeline;
    }

    VkPipelineLayout getVkPipelineLayout() {
        return vkPipelineLayout;
    }

    const DescriptorSetLayoutConfig& getDescSetLayoutConfig(int i) const;

    void init(VulkanContext& vkCxt, RenderPass* renderPass, DescriptorSetLayoutCache& layoutCache, glm::uvec2 extents);

    virtual VkPipelineLayout createPipelineLayout(VulkanContext& vkCxt, DescriptorSetLayoutCache& layoutCache) = 0;
    virtual VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) = 0;
    virtual void bind(VulkanContext& vkCxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) = 0;
};