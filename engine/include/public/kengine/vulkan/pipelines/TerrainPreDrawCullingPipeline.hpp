#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

class TerrainPreDrawCullingPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    struct PushConstant {
        uint32_t totalChunkCount;
    };

    inline static const DescriptorSetLayoutConfig preCullingLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT }
    };

    inline static const std::unique_ptr<Pipeline> create() {
        return std::make_unique<TerrainPreDrawCullingPipeline>();
    }

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};