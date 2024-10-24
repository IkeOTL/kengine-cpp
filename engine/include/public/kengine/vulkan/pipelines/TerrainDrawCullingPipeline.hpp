#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

namespace ke {
    class TerrainDrawCullingPipeline : public Pipeline {
    protected:
        void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

    public:
        struct PushConstant {
            glm::mat4 viewMatrix;
            std::array<float, 4> frustum;
            glm::vec4 sphereBounds;
            glm::uvec2 chunkDimensions;
            glm::uvec2 chunkCount;
            glm::vec2 worldOffset;
        };

        inline static const DescriptorSetLayoutConfig cullingLayout = {
            DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT },
            DescriptorSetLayoutBindingConfig{ 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT }
        };

        TerrainDrawCullingPipeline(VkDevice vkDevice)
            : Pipeline(vkDevice) {}

        inline static const std::unique_ptr<Pipeline> create(VkDevice vkDevice) {
            return std::make_unique<TerrainDrawCullingPipeline>(vkDevice);
        }

        VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
        VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
        void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
    };
} // namespace ke