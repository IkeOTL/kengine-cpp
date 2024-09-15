#include <kengine/vulkan/material/Material.hpp>
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <kengine/vulkan/descriptor/DescriptorSetAllocator.hpp>

const Pipeline& Material::getPipeline() const {
    return pipeline;
}

bool Material::hasShadow() const {
    return config->hasShadow();
}

bool Material::hasSkeleton()  const {
    return config->hasSkeleton();
}

const  MaterialBinding& Material::getBinding(int descSetIdx, int bindingIdx) const {
    auto it = materialBindings.find(descSetIdx);

    if (it == materialBindings.end())
        throw std::runtime_error("Material binding not found.");

    return *(it->second[bindingIdx]);
}

void Material::addBinding(std::unique_ptr<MaterialBinding>&& binding) {
    materialBindings[binding->getDescriptorSetIndex()].push_back(std::move(binding));
}

void Material::upload(VulkanContext& vkCxt, CachedGpuBuffer& buf, int frameIdx) const {
    config->upload(vkCxt, buf, frameIdx, id);
}

void Material::bindPipeline(VulkanContext& cxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) const {
    pipeline.bind(cxt, descSetAllocator, cmd, frameIndex);
}

void Material::bindMaterial(VulkanContext& cxt, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, uint32_t frameIndex) const {
    if (!materialBindings.size())
        return;

    for (auto& set : materialBindings) {
        auto& descLayoutConfig = pipeline.getDescSetLayoutConfig(set.first);
        auto pDescSet = descSetAllocator.leaseDescriptorSet(descLayoutConfig);
        auto& bindings = set.second;

        std::vector<uint32_t> offsets;
        offsets.reserve(bindings.size());

        // need better strategy for keeping these in scope
        std::vector<VkDescriptorBufferInfo> pBufferInfos;
        pBufferInfos.reserve(bindings.size());

        // need better strategy for keeping these in scope
        std::vector<VkDescriptorImageInfo> pImageInfos;
        pImageInfos.reserve(bindings.size());

        std::vector<VkWriteDescriptorSet> pDescWrites(bindings.size());
        for (int i = 0; i < bindings.size(); i++)
            bindings[i]->apply(cxt, frameIndex, pDescWrites[i], pDescSet,
                descLayoutConfig, pBufferInfos, pImageInfos, offsets);

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