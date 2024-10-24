#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/material/MaterialBindingConfig.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/texture/Texture2d.hpp>
#include <kengine/vulkan/SamplerCache.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetLayout.hpp>

namespace ke {
    int MaterialBinding::getDescriptorSetIndex() {
        return bindingConfig->getDescriptorSetIndex();
    }

    int MaterialBinding::getBindingIndex() {
        return bindingConfig->getBindingIndex();
    }

    void BufferBinding::apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
        VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
        std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) {
        auto& bindingConfig = layoutConfig.getBinding(getBindingIndex());

        // i dont like this, need to think of a more effecient way
        auto& bufferInfos = pBufferInfos.emplace_back(std::vector<VkDescriptorBufferInfo>{});
        bufferInfos.reserve(1);
        auto& bufferInfo = bufferInfos.emplace_back(VkDescriptorBufferInfo{});
        bufferInfo.buffer = gpuBuffer.getGpuBuffer().getVkBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = gpuBuffer.getFrameSize();

        setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.dstBinding = bindingConfig.bindingIndex;
        setWrite.dstSet = dstSet;
        setWrite.descriptorCount = bindingConfig.descriptorCount;
        setWrite.descriptorType = bindingConfig.descriptorType;
        setWrite.pBufferInfo = &bufferInfo;

        offsets.push_back(static_cast<uint32_t>(gpuBuffer.getFrameOffset(frameIdx)));
    }

    void ImageBinding::apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
        VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
        std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) {
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

        // i dont like this, need to think of a more effecient way
        auto& imageInfos = pImageInfos.emplace_back(std::vector<VkDescriptorImageInfo>{});
        imageInfos.reserve(1);
        auto& imageInfo = imageInfos.emplace_back(VkDescriptorImageInfo{});
        imageInfo.sampler = sampler;
        imageInfo.imageView = texture.getImageView();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.dstSet = dstSet;
        setWrite.dstBinding = bindingConfig.bindingIndex;
        setWrite.descriptorCount = bindingConfig.descriptorCount;
        setWrite.descriptorType = bindingConfig.descriptorType;
        setWrite.pImageInfo = &imageInfo;
    }

    void ImageArrayBinding::apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
        VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
        std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) {

        auto& imageInfos = pImageInfos.emplace_back(std::vector<VkDescriptorImageInfo>{});
        imageInfos.reserve(textures.size());

        for (auto* texture : textures) {
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
                texture->getMipLevels(),
                0,
                1.0f);
            auto sampler = cxt.getSamplerCache().getSampler(samplerConfig);


            // i dont like this, need to think of a more effecient way
            auto& imageInfo = imageInfos.emplace_back(VkDescriptorImageInfo{});
            imageInfo.sampler = sampler;
            imageInfo.imageView = texture->getImageView();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        auto& bindingConfig = layoutConfig.getBinding(getBindingIndex());
        setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.dstSet = dstSet;
        setWrite.dstBinding = bindingConfig.bindingIndex;
        setWrite.descriptorType = bindingConfig.descriptorType;
        setWrite.descriptorCount = textures.size();
        setWrite.pImageInfo = imageInfos.data();
    }
} // namespace ke