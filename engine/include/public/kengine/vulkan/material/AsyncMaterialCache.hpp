#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>

namespace ke {
    class Material;
    class PipelineCache;
    class AsyncTextureCache;
    class GpuBufferCache;

    class AsyncMaterialCache : public AsyncAssetCache<Material, MaterialConfig> {
    private:
        std::atomic<int> runningId;

        PipelineCache& pipelineCache;
        AsyncTextureCache& textureCache;
        GpuBufferCache& bufferCache;

    protected:
        std::unique_ptr<Material> create(std::shared_ptr<MaterialConfig> keyObj) override;

    public:

        /**
         * Since we add the material ID to the A channel of a R16G16B16A16 image we
         * only have [-1024, 1024] usable range without bit manipulation. This
         * should only be [-N, 0], because it's usecase is when unsigned ints are
         * unavailable.
         */
        static const int START_ID = 0;

        AsyncMaterialCache(PipelineCache& pipelineCache, AsyncTextureCache& textureCache, GpuBufferCache& bufferCache, ExecutorService& workerPool)
            : pipelineCache(pipelineCache), textureCache(textureCache), bufferCache(bufferCache), AsyncAssetCache(workerPool) {}

        inline static std::unique_ptr<AsyncMaterialCache> create(PipelineCache& pipelineCache, AsyncTextureCache& textureCache, GpuBufferCache& bufferCache, ExecutorService& workerPool) {
            return std::make_unique<AsyncMaterialCache>(pipelineCache, textureCache, bufferCache, workerPool);
        }
    };
} // namespace ke