#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VulkanContext.hpp>

class MaterialBindManager {
private:
    VulkanContext& vkCtx;
    VkPipeline lastPipeline;
    uint32_t lastMaterialId;

public:
    MaterialBindManager(VulkanContext& vkCtx)
        : vkCtx(vkCtx) {};

    void bind(Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
    void bindPipeline(Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex);
    void bindMaterial(Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, int frameIndex)
};