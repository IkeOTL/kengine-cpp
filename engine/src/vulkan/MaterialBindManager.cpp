#include <kengine/vulkan/MaterialBindManager.hpp>

namespace ke {
    void MaterialBindManager::reset() {
        lastPipeline = VK_NULL_HANDLE;
        lastMaterialId = -999999;
    }

    void MaterialBindManager::bind(const Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) {
        ZoneScoped;

        bindPipeline(mat, descSetAllocator, cmd, frameIndex);
        bindMaterial(mat, descSetAllocator, cmd, frameIndex);
    }

    void MaterialBindManager::bindPipeline(const Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) {
        if (lastPipeline != mat.getPipeline().getVkPipeline()) {
            mat.bindPipeline(vkCtx, descSetAllocator, cmd, frameIndex);
            lastPipeline = mat.getPipeline().getVkPipeline();
            //            System.out.println(frameIndex + " " + lastPipeline);
        }
    }

    void MaterialBindManager::bindMaterial(const Material& mat, DescriptorSetAllocator& descSetAllocator, VkCommandBuffer cmd, size_t frameIndex) {
        // need to be more sophisticated with material binding
        // for example take in to account sampler and possible UBO stuff
        if (lastMaterialId != mat.getId()) {
            mat.bindMaterial(vkCtx, descSetAllocator, cmd, frameIndex);
            lastMaterialId = mat.getId();
            //            System.out.println(frameIndex + " " + lastMaterialId);
        }
    }
} // namespace ke
