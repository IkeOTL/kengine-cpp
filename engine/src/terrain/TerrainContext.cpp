#include <kengine/terrain/TerrainContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/vulkan/pipelines/TerrainDeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/pipelines/TerrainDrawCullingPipeline.hpp>


const size_t TerrainContext::terrainDataBufAlignedFrameSize(VulkanContext& vkCxt) {
    return vkCxt.alignSsboFrame(TerrainContext::MAX_TILES * sizeof(uint32_t));
}

const size_t TerrainContext::drawInstanceBufAlignedFrameSize(VulkanContext& vkCxt) {
    return vkCxt.alignSsboFrame(TerrainContext::MAX_CHUNKS * sizeof(uint32_t));
}

void TerrainContext::init(VulkanContext& vkCxt, std::vector<std::unique_ptr<DescriptorSetAllocator>>& descSetAllocators) {
    auto tilesWidth = 64;
    auto tilesLength = 64;
    // terrain

    terrain = std::make_unique<DualGridTileTerrain>(tilesWidth, tilesLength, 16, 16);

    /*auto matConfig = PbrMaterialConfig::create();
    TextureConfig textureConfig("img/poke-tileset.png");
    matConfig->setHasShadow(false);
    matConfig->addAlbedoTexture(&textureConfig);
    matConfig->setMetallicFactor(0.0f);
    matConfig->setRoughnessFactor(0.5f);*/

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
    auto cmdBuf = static_cast<VkDrawIndexedIndirectCommand*>(drawIndirectCmdBuf->getGpuBuffer().data());
    for (auto i = 0; i < VulkanContext::FRAME_OVERLAP; i++)
        memcpy(cmdBuf + i, &cmd, sizeof(VkDrawIndexedIndirectCommand));

    // probably change to a device only buffer?
    terrainDataBuf = &bufCache.createHostMapped(
        MAX_TILES * sizeof(uint32_t),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
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
        setWrites.reserve(5);
        bufferInfos.reserve(5);

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
                &materialsBuf
            );
        }

        {
            auto set0 = descSetAllocator->getGlobalDescriptorSet("terrain-deferred-culling", TerrainDrawCullingPipeline::cullingLayout);

            pushBuf(
                set0,
                TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(0),
                drawIndirectCmdBuf
            );

            pushBuf(
                set0,
                TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(1),
                drawInstanceBuf
            );
        }

        vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
    }
}

void TerrainContext::resetDrawBuf(uint32_t frameIdx) {
    assert(frameIdx >= 0 && frameIdx <= VulkanContext::FRAME_OVERLAP);

    auto cmd = static_cast<VkDrawIndexedIndirectCommand*>(drawIndirectCmdBuf->getGpuBuffer().data());
    cmd += frameIdx;
    cmd->instanceCount = 0;
}

// TODO: optimize this change to ref once we have it calced once
//const glm::vec4& TerrainContext::getChunkBoundingSphere() {
const glm::vec4 TerrainContext::getChunkBoundingSphere() {
    glm::vec3 offset = glm::vec3{ terrain->getChunkWidth() * 0.5f, 0, terrain->getChunkLength() * 0.5f };
    float radius = glm::length(offset);
    return glm::vec4{ offset, radius };
}

// TODO: optimize this change to ref once we have it calced once
// need to update tileterrain to return ref to vecs for dimensions and stuff
const glm::uvec2 TerrainContext::getChunkCount() {
    return glm::uvec2{ terrain->getChunkCountX() ,terrain->getChunkCountZ() };
}

const glm::uvec2 TerrainContext::getChunkDimensions() {
    return glm::uvec2{ terrain->getChunkWidth() ,terrain->getChunkLength() };
}

const glm::vec2 TerrainContext::getWorldOffset() {
    return glm::uvec2{ terrain->getWorldOffsetX() ,terrain->getWorldOffsetZ() };
}
void TerrainContext::draw(VulkanContext& vkCxt, RenderPassContext& rpCtx, DescriptorSetAllocator& descSetAllocator) {
    auto pc = TerrainDeferredOffscreenPbrPipeline::PushConstant{};
    pc.chunkDimensions = getChunkDimensions();
    pc.chunkCount = getChunkCount();
    pc.tilesheetDimensions = glm::uvec2{ 96, 80 };
    pc.tileDimensions = glm::uvec2{ 16, 16 };
    pc.materialIds = glm::uvec4{ 0 };
    pc.worldOffset = getWorldOffset();

    auto& pl = vkCxt.getPipelineCache().getPipeline<TerrainDeferredOffscreenPbrPipeline>();
    pl.bind(vkCxt, descSetAllocator, rpCtx.cmd, rpCtx.renderTargetIndex);
    vkCmdPushConstants(rpCtx.cmd, pl.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TerrainDeferredOffscreenPbrPipeline::PushConstant), &pc);

    vkCmdBindIndexBuffer(rpCtx.cmd, chunkIndicesBuf->vkBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexedIndirect(
        rpCtx.cmd,
        drawIndirectCmdBuf->getGpuBuffer().getVkBuffer(),
        0,
        1,
        sizeof(VkDrawIndexedIndirectCommand));
}