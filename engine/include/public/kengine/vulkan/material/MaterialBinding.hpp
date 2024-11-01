#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

#include <memory>
#include <future>

namespace ke {
    class MaterialBindingConfig;
    class GpuBuffer;
    class CachedGpuBuffer;
    class Texture2d;
    class VulkanContext;
    class DescriptorSetLayoutConfig;

    class MaterialBinding {
    private:
        const std::shared_ptr<MaterialBindingConfig> bindingConfig;

    public:
        MaterialBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig)
            : bindingConfig(bindingConfig) {}

        int getDescriptorSetIndex();
        int getBindingIndex();

        virtual void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
            VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
            std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) = 0;
    };

    class BufferBinding : public MaterialBinding {
    private:
        CachedGpuBuffer& gpuBuffer;

    public:
        BufferBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, CachedGpuBuffer& gpuBuffer)
            : MaterialBinding(bindingConfig),
              gpuBuffer(gpuBuffer) {}

        const CachedGpuBuffer& getGpuBuffer() const {
            return gpuBuffer;
        }

        void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
            VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
            std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) override;
    };

    class ImageBinding : public MaterialBinding {
    private:
        Texture2d& texture;

    public:
        ImageBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, Texture2d& texture)
            : MaterialBinding(bindingConfig),
              texture(texture) {}

        const Texture2d& getTexture() const {
            return texture;
        }

        void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
            VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
            std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) override;
    };

    class ImageArrayBinding : public MaterialBinding {
    private:
        std::vector<Texture2d*> textures;

    public:
        ImageArrayBinding(std::shared_ptr<MaterialBindingConfig> bindingConfig, std::vector<Texture2d*> textures)
            : MaterialBinding(bindingConfig),
              textures(textures) {}

        const std::vector<Texture2d*> getTextures() const {
            return textures;
        }

        void apply(VulkanContext& cxt, int frameIdx, VkWriteDescriptorSet& setWrite,
            VkDescriptorSet dstSet, const DescriptorSetLayoutConfig& layoutConfig,
            std::vector<std::vector<VkDescriptorBufferInfo>>& pBufferInfos, std::vector<std::vector<VkDescriptorImageInfo>>& pImageInfos, std::vector<uint32_t>& offsets) override;
    };
} // namespace ke
