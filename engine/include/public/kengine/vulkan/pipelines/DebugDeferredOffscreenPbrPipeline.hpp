#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

class DebugDeferredOffscreenPbrPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    inline static const DescriptorSetLayoutConfig objectLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
    };

    struct PushConstant {
        glm::vec4 color;
    };

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};