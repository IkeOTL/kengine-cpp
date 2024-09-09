#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

class DeferredOffscreenPbrPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    inline static const DescriptorSetLayoutConfig objectLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 2, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT },
    };
    inline static const DescriptorSetLayoutConfig pbrTextureLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 2, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 3, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 4, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
    };

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};