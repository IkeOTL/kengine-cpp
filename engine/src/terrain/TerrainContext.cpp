#include <kengine/terrain/TerrainContext.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/vulkan/pipelines/TerrainDeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/CameraController.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/vulkan/pipelines/TerrainDrawCullingPipeline.hpp>
#include <kengine/vulkan/texture/TextureConfig.hpp>
#include <kengine/vulkan/material/TerrainPbrMaterialConfig.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/vulkan/material/Material.hpp>
#include <kengine/util/Random.hpp>
#include <kengine/vulkan/SamplerCache.hpp>
#include <kengine/terrain/OptimizedTerrain.hpp>

namespace ke {
    const size_t TerrainContext::terrainDataBufAlignedFrameSize(VulkanContext& vkCxt) {
        return vkCxt.alignSsboFrame(TerrainContext::MAX_TILES * sizeof(uint32_t));
    }

    const size_t TerrainContext::drawInstanceBufAlignedFrameSize(VulkanContext& vkCxt) {
        return vkCxt.alignSsboFrame(TerrainContext::MAX_CHUNKS * sizeof(uint32_t));
    }

    void TerrainContext::init(VulkanContext& vkCxt) {
        auto tilesWidth = 128;
        auto tilesLength = 128;
        // terrain

        terrain = std::make_unique<OptimizedTerrain>(tilesWidth, tilesLength, 16, 16);

        auto matConfig = TerrainPbrMaterialConfig::create();
        TextureConfig textureConfig("img/poke-tileset.png");
        matConfig->setHasShadow(false);
        matConfig->addAlbedoTextures({textureConfig});
        matConfig->setMetallicFactor(0.0f);
        matConfig->setRoughnessFactor(0.5f);

        material = &materialCache.get(matConfig);

        auto& bufCache = vkCxt.getGpuBufferCache();

        // chunkIndicesBuf;
        {
            std::vector<uint32_t> indices;

            /*
                verts     faces
               0----3    1---0  0
               |    |    |  / / |
               |    |    | / /  |
               1----2    2  1---2
             */
            uint32_t curVert = 0;
            for (int z = 0; z < terrain->getChunkLength(); z++) {
                for (int x = 0; x < terrain->getChunkWidth(); x++) {
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

            // probably change to a device only buffer?
            {
                VkDrawIndexedIndirectCommand cmd{};
                cmd.indexCount = indices.size();

                drawIndirectCmdBuf = &bufCache.upload(
                    [&cmd](VulkanContext& vkCxt, void* data, VkDeviceSize frameSize, uint32_t frameCount) {
                        for (auto i = 0; i < frameCount; i++) {
                            auto pos = frameSize * i;
                            memcpy(static_cast<unsigned char*>(data) + pos, &cmd, sizeof(VkDrawIndexedIndirectCommand));
                        }
                    },
                    sizeof(VkDrawIndexedIndirectCommand), VulkanContext::FRAME_OVERLAP,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
                    VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
                    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    0);
            }

            chunkIndicesBuf = &bufCache.upload(
                [&indices](VulkanContext& vkCxt, void* data, VkDeviceSize frameSize, uint32_t frameCount) {
                    memcpy(data, indices.data(), indices.size() * sizeof(uint32_t));
                },
                indices.size() * sizeof(uint32_t), 1,
                VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
                VK_ACCESS_2_INDEX_READ_BIT,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                0);
        }

        // probably change to a device only buffer?
        terrainDataBuf = &bufCache.createHostMapped(
            MAX_TILES * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

        // fill in tiledata
        {
            auto i = 0;
            for (auto chunkZ = 0; chunkZ < terrain->getChunkCountZ(); chunkZ++) {
                for (auto chunkX = 0; chunkX < terrain->getChunkCountX(); chunkX++) {
                    auto startX = chunkX * terrain->getChunkWidth();
                    auto startZ = chunkZ * terrain->getChunkLength();
                    // set entire chunk to a single tile for visual verification
                    uint16_t tileId = random::randInt(0, 2); // id of the tile in the tilesheet
                    for (auto z = 0; z < terrain->getChunkLength(); z++) {
                        for (auto x = 0; x < terrain->getChunkWidth(); x++) {
                            i++;
                            uint8_t matIdx = 0;

                            auto& tile = terrain->getTile(startX + x, startZ + z);
                            tile.setMatIdx(0);
                            tile.setTileId(tileId);
                        }
                    }
                }
            }

            auto& tileData = terrain->getTileData();
            auto* buf = static_cast<uint32_t*>(terrainDataBuf->getGpuBuffer().data());
            memcpy(buf, terrain->getTileData().data(), terrain->getTileData().size() * sizeof(uint32_t));
        }

        // generate heights
        {
            auto vertexCountX = terrain->getTerrainHeightsWidth();
            auto vertexCountZ = terrain->getTerrainHeightsLength();

            // a "unit" will be 10
            float max = 0xFF;
            auto randMax = .3f;
            for (auto z = 0; z < terrain->getTerrainHeightsLength(); z++) {
                for (auto x = 0; x < terrain->getTerrainHeightsWidth(); x++) {
                    auto f0 = random::randFloat(0, randMax);
                    terrain->setHeight(x, z, f0);
                }
            }

            // set obvious points for visual confirmation
            {
                auto area = 6;
                for (size_t z = 0; z < area; z++) {
                    for (auto x = 0; x < area; x++) {
                        auto posX = x + (vertexCountX / 2) - (area / 2);
                        auto posZ = z + (vertexCountZ / 2) - (area / 2);
                        terrain->setHeight(posX, posZ, 3);
                    }
                }

                // depress center
                area = 2;
                for (size_t z = 0; z < area; z++) {
                    for (auto x = 0; x < area; x++) {
                        auto posX = x + (vertexCountX / 2) - (area / 2);
                        auto posZ = z + (vertexCountZ / 2) - (area / 2);
                        terrain->setHeight(posX, posZ, 0);
                    }
                }
            }

            /*  Texture2d(VulkanContext& vkCxt, const unsigned char* image, uint32_t width, uint32_t height,
                  VkFormat format, VkImageType imageType, VkImageViewType imageViewType, int channels,
                  VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccesMask, bool generateMipMaps)*/
            heightsTexture = std::make_unique<Texture2d>(
                vkCxt,
                terrain->getHeights().data(),
                vertexCountX,
                vertexCountZ,
                VK_FORMAT_R8_UNORM,
                VK_IMAGE_TYPE_2D,
                VK_IMAGE_VIEW_TYPE_2D,
                1,
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                false);
        }

        drawInstanceBuf = &bufCache.create(
            MAX_CHUNKS * sizeof(uint32_t),
            VulkanContext::FRAME_OVERLAP,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

        auto& descSetAllocators = vkCxt.getDescSetAllocators();
        for (int i = 0; i < VulkanContext::FRAME_OVERLAP; i++) {
            auto& descSetAllocator = descSetAllocators[i];

            std::vector<VkWriteDescriptorSet> setWrites;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;

            // so they dont resize
            setWrites.reserve(6);
            imageInfos.reserve(1);
            bufferInfos.reserve(5);

            auto pushBuf = [&](VkDescriptorSet vkDescSet, const DescriptorSetLayoutBindingConfig& bindingCfg, CachedGpuBuffer* gpuBuf) -> void {
                auto& buf = bufferInfos.emplace_back(VkDescriptorBufferInfo{
                    gpuBuf->getGpuBuffer().vkBuffer,
                    0,
                    gpuBuf->getFrameSize()});

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
                    nullptr});
            };

            auto pushImg = [&](VkDescriptorSet vkDescSet, const DescriptorSetLayoutBindingConfig& bindingCfg, const VkImageView imgView, VkSampler sampler) -> void {
                auto& img = imageInfos.emplace_back(VkDescriptorImageInfo{
                    sampler,
                    imgView,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});

                setWrites.emplace_back(VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    vkDescSet,
                    bindingCfg.bindingIndex,
                    0,
                    bindingCfg.descriptorCount,
                    bindingCfg.descriptorType,
                    &img,
                    nullptr,
                    nullptr});
            };

            {
                auto deferredDescriptorSet = descSetAllocator->getGlobalDescriptorSet("terrain-deferred-gbuffer", TerrainDeferredOffscreenPbrPipeline::objectLayout);

                pushBuf(
                    deferredDescriptorSet,
                    TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(0),
                    terrainDataBuf);

                {
                    auto samplerConfig = SamplerConfig(
                        VK_SAMPLER_MIPMAP_MODE_LINEAR,
                        VK_FILTER_LINEAR,
                        VK_FILTER_LINEAR,
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                        VK_COMPARE_OP_NEVER,
                        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                        0,
                        1,
                        0,
                        1.0f);

                    auto sampler = vkCxt.getSamplerCache().getSampler(samplerConfig);

                    pushImg(
                        deferredDescriptorSet,
                        TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(1),
                        heightsTexture->getImageView(),
                        sampler);
                }

                pushBuf(
                    deferredDescriptorSet,
                    TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(2),
                    drawInstanceBuf);

                pushBuf(
                    deferredDescriptorSet,
                    TerrainDeferredOffscreenPbrPipeline::objectLayout.getBinding(3),
                    materialsBuf);
            }

            {
                auto set0 = descSetAllocator->getGlobalDescriptorSet("terrain-deferred-culling", TerrainDrawCullingPipeline::cullingLayout);

                pushBuf(
                    set0,
                    TerrainDrawCullingPipeline::cullingLayout.getBinding(0),
                    drawIndirectCmdBuf);

                pushBuf(
                    set0,
                    TerrainDrawCullingPipeline::cullingLayout.getBinding(1),
                    drawInstanceBuf);
            }

            vkUpdateDescriptorSets(vkCxt.getVkDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
        }
    }

    // TODO: optimize this change to ref once we have it calced once
    // const glm::vec4& TerrainContext::getChunkBoundingSphere() {
    const glm::vec4 TerrainContext::getChunkBoundingSphere() {
        glm::vec3 offset = glm::vec3{terrain->getChunkWidth() * 0.5f, 0, terrain->getChunkLength() * 0.5f};
        float radius = glm::length(offset);
        return glm::vec4{offset, radius};
    }

    // TODO: optimize this change to ref once we have it calced once
    // need to update tileterrain to return ref to vecs for dimensions and stuff
    const glm::uvec2 TerrainContext::getChunkCount() {
        return glm::uvec2{terrain->getChunkCountX(), terrain->getChunkCountZ()};
    }

    const glm::uvec2 TerrainContext::getChunkDimensions() {
        return glm::uvec2{terrain->getChunkWidth(), terrain->getChunkLength()};
    }

    const glm::vec2 TerrainContext::getWorldOffset() {
        return glm::vec2{terrain->getWorldOffsetX(), terrain->getWorldOffsetZ()};
    }
    void TerrainContext::draw(VulkanContext& vkCxt, RenderPassContext& rpCtx, DescriptorSetAllocator& descSetAllocator) {
        auto& pl = vkCxt.getPipelineCache().getPipeline<TerrainDeferredOffscreenPbrPipeline>();
        auto frameIdx = rpCtx.renderTargetIndex;

        material->upload(vkCxt, *materialsBuf, frameIdx);
        material->bindPipeline(vkCxt, descSetAllocator, rpCtx.cmd, frameIdx);
        material->bindMaterial(vkCxt, descSetAllocator, rpCtx.cmd, frameIdx);

        auto pc = TerrainDeferredOffscreenPbrPipeline::PushConstant{};
        pc.chunkDimensions = getChunkDimensions();
        pc.chunkCount = getChunkCount();
        pc.tilesheetDimensions = glm::uvec2{96, 80};
        pc.tileDimensions = glm::uvec2{16, 16};
        pc.materialIds = glm::uvec4{material->getId()};
        pc.worldOffset = getWorldOffset();
        pc.tileUvSize = glm::vec2{16.0f / 96.0f, 16.0f / 80.0f};
        pc.tileDenom = static_cast<uint32_t>(96.0f / 16.0f);
        pc.vertHeightFactor = glm::vec4{
            terrain->getTerrainHeightsWidth() * 0.5f,
            terrain->getTerrainHeightsLength() * 0.5f,
            1.0f / terrain->getTerrainHeightsWidth(),
            1.0f / terrain->getTerrainHeightsLength()};

        vkCmdPushConstants(rpCtx.cmd, pl.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TerrainDeferredOffscreenPbrPipeline::PushConstant), &pc);

        vkCmdBindIndexBuffer(rpCtx.cmd, chunkIndicesBuf->getGpuBuffer().vkBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexedIndirect(
            rpCtx.cmd,
            drawIndirectCmdBuf->getGpuBuffer().getVkBuffer(),
            drawIndirectCmdBuf->getFrameOffset(frameIdx),
            1,
            sizeof(VkDrawIndexedIndirectCommand));
    }
} // namespace ke