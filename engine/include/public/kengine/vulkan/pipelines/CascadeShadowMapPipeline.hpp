#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>

class CascadeShadowMapPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    struct PushConstant {
        uint32_t cascadeIndex;
    };

    inline static const DescriptorSetLayoutConfig shadowPassLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
    };

    inline static const DescriptorSetLayoutConfig cascadeViewProjLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
    };

    inline static const DescriptorSetLayoutConfig textureLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT }
    };

    inline static const std::unique_ptr<Pipeline> create() {
        return std::make_unique<CascadeShadowMapPipeline>();
    }

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};