#include <kengine/vulkan/material/Material.hpp>
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>

Pipeline& Material::getPipeline() {
    return pipeline;
}

boolean Material::hasShadow() {
    return config->hasShadow();
}

boolean Material::hasSkeleton() {
    return config->hasSkeleton();
}

MaterialBinding& Material::getBinding(int descSetIdx, int bindingIdx) {
    auto it = materialBindings.find(descSetIdx);

    if (it == materialBindings.end())
        throw std::runtime_error("Material binding not found.");

    return *(it->second[bindingIdx]);
}

void Material::addBinding(std::unique_ptr<MaterialBinding>&& binding) {
    materialBindings[binding->getDescriptorSetIndex()].push_back(std::move(binding));
}

void Material::upload(VulkanContext& vkCxt, CachedGpuBuffer& buf, int frameIdx) {
    config->upload(vkCxt, buf, frameIdx, id);
}

void Material::bindPipeline(VulkanContext& cxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) {
    pipeline.bind(cxt, descSetAllocator, cmd, frameIndex);
}

void Material::bindMaterial(VulkanContext& cxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) {
    if (!materialBindings.size())
        return;

    for (auto& set : materialBindings) {
        auto& descLayoutConfig = pipeline.getDescSetLayoutConfig(set.first);
        auto pDescSet = descSetAllocator.leaseDescriptorSet(descLayoutConfig);
        auto& bindings = set.second;

        std::vector<uint32_t> offsets(bindings.size());

        std::vector<VkWriteDescriptorSet> pDescWrites(bindings.size());
        for (int i = 0; i < bindings.size(); i++) {
            auto& binding = bindings[i];
            auto& setWrite = pDescWrites[i];
            binding->apply(cxt, frameIndex, setWrite, pDescSet, descLayoutConfig, offsets);
        }

        // move outside loop so we apply all updates once?
        vkUpdateDescriptorSets(cxt.getVkDevice(), pDescWrites.size(), pDescWrites.data(), 0, nullptr);
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline.getVkPipelineLayout(),
            set.first,
            1,
            &pDescSet,
            offsets.size(),
            offsets.data()
        );
    }
}