#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

class SkinnedOffscreenPbrPipeline : public Pipeline {
protected:
    void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

public:
    const static DescriptorSetLayoutConfig pbrSkinnedTextureLayout;

    VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
    VkPipeline createPipeline(VkDevice device, RenderPass& renderPass, VkPipelineLayout pipelineLayout, glm::uvec2  extents) override;
    void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) override;
};