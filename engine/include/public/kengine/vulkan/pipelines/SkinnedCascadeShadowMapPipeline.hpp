#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>

class SkinnedCascadeShadowMapPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    inline static const DescriptorSetLayoutConfig skinnedSingleTextureLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
    };

    SkinnedCascadeShadowMapPipeline(VkDevice vkDevice)
        : Pipeline(vkDevice) {}

    inline static const std::unique_ptr<Pipeline> create(VkDevice vkDevice) {
        return std::make_unique<SkinnedCascadeShadowMapPipeline>(vkDevice);
    }

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};