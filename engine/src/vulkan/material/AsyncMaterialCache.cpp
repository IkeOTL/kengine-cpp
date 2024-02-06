#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/vulkan/material/Material.hpp>
#include <memory>

std::unique_ptr<Material> AsyncMaterialCache::create(MaterialConfig key)
{
    auto newMat = std::make_unique<Material>(runningId++, key, pipelineCache->getPipeline(key.getPipeline()));

    auto& tConfigs = key.getBindingConfigs();
    for (auto& entry : tConfigs) {
        auto& c = entry.second;
        // fine to join. this func runs in the task so it will block the task's thread
        auto binding = c->getBinding(*textureCache, *bufferCache).get();
        newMat->addBinding(std::move(binding));
    }

    return newMat;
}
