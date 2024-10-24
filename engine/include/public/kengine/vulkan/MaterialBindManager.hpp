#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include "material/Material.hpp"

namespace ke {
    class MaterialBindManager {
    private:
        VulkanContext& vkCtx;
        VkPipeline lastPipeline = VK_NULL_HANDLE;
        uint32_t lastMaterialId = -9999999;

    public:
        MaterialBindManager(VulkanContext& vkCtx)
            : vkCtx(vkCtx) {};

        void reset();
        void bind(const Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
        void bindPipeline(const Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
        void bindMaterial(const Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
    };
} // namespace ke