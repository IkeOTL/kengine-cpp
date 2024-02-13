#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include "material/Material.hpp"

class MaterialBindManager {
private:
    VulkanContext& vkCtx;
    VkPipeline lastPipeline = VK_NULL_HANDLE;
    uint32_t lastMaterialId = 0;

public:
    MaterialBindManager(VulkanContext& vkCtx)
        : vkCtx(vkCtx) {};

    void bind(Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
    void bindPipeline(Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
    void bindMaterial(Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
};