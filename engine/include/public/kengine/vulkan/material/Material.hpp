#pragma once
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>

class VulkanContext;
class CachedGpuBuffer;
class MaterialConfig;
class Pipeline;
class DescriptorSetAllocator;

class Material {

private:
    const uint32_t id;
    std::shared_ptr<MaterialConfig> config;

    Pipeline& pipeline;
    std::unordered_map<int, std::vector<std::unique_ptr<MaterialBinding>>> materialBindings;

public:
    Material(const uint32_t id, std::shared_ptr<MaterialConfig> config, Pipeline& pipeline)
        : id(id), config(config), pipeline(pipeline) {}

    uint32_t getId() const {
        return id - AsyncMaterialCache::START_ID;
    }

    const Pipeline& getPipeline() const;

    boolean hasShadow() const;
    boolean hasSkeleton() const;

    const MaterialBinding& getBinding(int descSetIdx, int bindingIdx) const;
    void addBinding(std::unique_ptr<MaterialBinding>&& binding);
    void upload(VulkanContext& vkCxt, CachedGpuBuffer& buf, int frameIdx) const;
    void bindPipeline(VulkanContext& cxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) const;
    void bindMaterial(VulkanContext& cxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) const;
};