#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/material/Material.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <kengine/vulkan/texture/AsyncTextureCache.hpp>
#include <kengine/vulkan/pipelines/PipelineCache.hpp>

class AsyncMaterialCache : AsyncAssetCache<Material, MaterialConfig> {
private:
    std::atomic<int> runningId;

    std::shared_ptr<PipelineCache> pipelineCache;
    std::shared_ptr<AsyncTextureCache> textureCache;
    std::shared_ptr<GpuBufferCache> bufferCache;

public:

    /**
     * Since we add the material ID to the A channel of a R16G16B16A16 image we
     * only have [-1024, 1024] usable range without bit manipulation. This
     * should only be [-N, 0], because it's usecase is when unsigned ints are
     * unavailable.
     */
    static const int START_ID = 0;

    AsyncMaterialCache(ExecutorService& workerPool)
        : AsyncAssetCache(workerPool) {}
};