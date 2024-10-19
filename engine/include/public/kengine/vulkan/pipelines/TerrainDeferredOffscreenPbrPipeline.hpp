#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

class TerrainDeferredOffscreenPbrPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    struct PushConstant {;
        glm::uvec2 chunkDimensions;
        glm::uvec2 chunkCount;
        glm::uvec2 tilesheetDimensions;
        glm::uvec2 tileDimensions;
        glm::uvec4 materialIds;
        glm::vec2 worldOffset;
        glm::vec2 tileUvSize;
        uint32_t tileDenom;
    };

    inline static const DescriptorSetLayoutConfig objectLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 2, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT },
        DescriptorSetLayoutBindingConfig{ 3, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT },
    };
    inline static const DescriptorSetLayoutConfig pbrTextureLayout = {
        DescriptorSetLayoutBindingConfig{ 0, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 1, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 2, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 3, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
        DescriptorSetLayoutBindingConfig{ 4, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT },
    };

    inline static const std::unique_ptr<Pipeline> create() {
        return std::make_unique<TerrainDeferredOffscreenPbrPipeline>();
    }

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
};