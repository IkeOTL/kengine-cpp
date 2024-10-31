#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

namespace ke {
    class DebugDeferredOffscreenPbrPipeline : public Pipeline {
    protected:
        void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

    public:
        struct PushConstant {
            glm::mat4 transform;
            glm::vec4 color;
        };

        DebugDeferredOffscreenPbrPipeline(VkDevice vkDevice)
            : Pipeline(vkDevice) {}

        VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
        VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) override;
        void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
    };
} // namespace ke