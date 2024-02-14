#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>

class CascadeShadowMapPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    struct PushConstant {
        uint32_t cascadeIndex;
    };

    const static DescriptorSetLayoutConfig shadowPassLayout;
    const static DescriptorSetLayoutConfig cascadeViewProjLayout;
    const static DescriptorSetLayoutConfig textureLayout;

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass& renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};