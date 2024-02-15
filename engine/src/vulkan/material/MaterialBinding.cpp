#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>
#include <kengine/vulkan/SamplerCache.hpp>

int MaterialBinding::getDescriptorSetIndex() {
    return bindingConfig->getDescriptorSetIndex();
}

int MaterialBinding::getBindingIndex() {
    return bindingConfig->getBindingIndex();
}

void BufferBinding::apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet setWrite, VkDescriptorSet dstSet,
    const DescriptorSetLayoutConfig& layoutConfig, std::vector<uint32_t> offsets) {
    auto& bindingConfig = layoutConfig.getBinding(getBindingIndex());

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = gpuBuffer.getGpuBuffer().getVkBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = gpuBuffer.getFrameSize();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding = bindingConfig.bindingIndex;
    descriptorWrite.dstSet = dstSet;
    descriptorWrite.descriptorCount = bindingConfig.descriptorCount;
    descriptorWrite.descriptorType = bindingConfig.descriptorType;
    descriptorWrite.pBufferInfo = &bufferInfo;

    offsets.push_back(static_cast<uint32_t>(gpuBuffer.getFrameOffset(frameIdx)));
}

void ImageBinding::apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet setWrite, VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig, std::vector<uint32_t> offsets) {
    auto samplerConfig = SamplerConfig(
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_FILTER_NEAREST,
        VK_FILTER_NEAREST,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_COMPARE_OP_NEVER,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        0,
        texture.getMipLevels(),
        0,
        1.0f);
    auto sampler = cxt.getSamplerCache().getSampler(samplerConfig);

    auto& bindingConfig = layoutConfig.getBinding(getBindingIndex());

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler;
    imageInfo.imageView = texture.getImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = dstSet;
    descriptorWrite.dstBinding = bindingConfig.bindingIndex;
    descriptorWrite.descriptorCount = bindingConfig.descriptorCount;
    descriptorWrite.descriptorType = bindingConfig.descriptorType;
    descriptorWrite.pImageInfo = &imageInfo;
}
