#pragma once
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <glm/mat4x4.hpp>
#include <array>

namespace ke {
    class DeferredCompositionPbrPipeline : public Pipeline {
    protected:
        void loadDescriptorSetLayoutConfigs(std::vector<DescriptorSetLayoutConfig>& dst) override;

    public:
        inline static const DescriptorSetLayoutConfig compositionLayout = {
            DescriptorSetLayoutBindingConfig{0, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{1, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{2, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{3, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{4, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{5, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{6, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{7, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT},
            DescriptorSetLayoutBindingConfig{8, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT}};

        DeferredCompositionPbrPipeline(VkDevice vkDevice)
            : Pipeline(vkDevice) {}

        inline static const std::unique_ptr<Pipeline> create(VkDevice vkDevice) {
            return std::make_unique<DeferredCompositionPbrPipeline>(vkDevice);
        }

        VkPipelineLayout createPipelineLayout(VulkanContext& vkContext, DescriptorSetLayoutCache& layoutCache) override;
        VkPipeline createPipeline(VkDevice device, RenderPass* renderPass, VkPipelineLayout pipelineLayout, glm::uvec2 extents) override;
        void bind(VulkanContext& engine, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) override;
    };
} // namespace ke