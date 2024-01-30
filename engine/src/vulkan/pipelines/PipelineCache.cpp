#include <kengine/vulkan/pipelines/PipelineCache.hpp>

DescriptorSetLayoutConfig globalLayout = {
    DescriptorSetLayoutBindingConfig{ 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT }
};