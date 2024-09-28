#include <kengine/terrain/TerrainContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/vulkan/pipelines/TerrainDeferredOffscreenPbrPipeline.hpp>


void TerrainContext::init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators) {
    auto& bufCache = vkCxt.getGpuBufferCache();

    auto xferFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        //| VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    //chunkIndicesBuf;
    {
        auto chunkWidth = 64;
        auto chunkLength = 64;

        std::vector<uint32_t> indices;

        /*
            verts     faces
           0----3    1---0  0
           |    |    |  / / |
           |    |    | / /  |
           1----2    2  1---2
         */
        uint32_t curVert = 0;
        for (int z = 0; z < chunkLength; z++) {
            for (int x = 0; x < chunkWidth; x++) {
                auto i0 = curVert++;
                auto i1 = curVert++;
                auto i2 = curVert++;
                auto i3 = curVert++;

                indices.emplace_back(i3);
                indices.emplace_back(i0);
                indices.emplace_back(i1);

                indices.emplace_back(i3);
                indices.emplace_back(i1);
                indices.emplace_back(i2);
            }
        }

        auto idxBuffer = std::make_unique<IndexBuffer>(indices);
        vkCxt.uploadBuffer(*idxBuffer,
            VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_ACCESS_2_INDEX_READ_BIT,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT, xferFlags, nullptr);

        chunkIndicesBuf = std::move(idxBuffer->releaseBuffer());
    }

    // probably change to a device only buffer?
    drawIndirectCmdBuf = &bufCache.createHostMapped(
        sizeof(VkDrawIndexedIndirectCommand),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);
    VkDrawIndexedIndirectCommand cmd{};
    memcpy(drawIndirectCmdBuf->getGpuBuffer().data(), &cmd, sizeof(VkDrawIndexedIndirectCommand));

    // probably change to a device only buffer?
    terrainDataBuf = &bufCache.createHostMapped(
        MAX_TILES * sizeof(uint32_t),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);

    drawInstanceBuf = &bufCache.create(
        MAX_CHUNKS * sizeof(uint32_t),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);


    for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
        auto& descSetAllocator = descSetAllocators[i];

        std::vector<VkWriteDescriptorSet> setWrites;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;

        // so they dont resize
        setWrites.reserve(3);
        bufferInfos.reserve(3);

        auto pushBuf = [&](VkDescriptorSet vkDescSet, const DescriptorSetLayoutBindingConfig& bindingCfg, CachedGpuBuffer* gpuBuf) -> void {
            auto& buf = bufferInfos.emplace_back(VkDescriptorBufferInfo{
                    gpuBuf->getGpuBuffer().vkBuffer,
                    0,
                    gpuBuf->getFrameSize()
                });

            setWrites.emplace_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    vkDescSet,
                    bindingCfg.bindingIndex,
                    0,
                    bindingCfg.descriptorCount,
                    bindingCfg.descriptorType,
                    nullptr,
                    &buf,
                    nullptr
                });
            };

        {
            auto deferredDescriptorSet = descSetAllocator->getGlobalDescriptorSet("terrain-deferred-gbuffer", TerrainDeferredOffscreenPbrPipeline::objectLayout);

            pushBuf(
                deferredDescriptorSet,
                TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(0),
                terrainDataBuf
            );

            pushBuf(
                deferredDescriptorSet,
                TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(1),
                drawInstanceBuf
            );

            pushBuf(
                deferredDescriptorSet,
                TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(2),
                materialsBuf
            );
        }

        vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
    }
}