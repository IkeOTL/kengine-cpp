#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/vulkan/material/Material.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <memory>

std::unique_ptr<Material> AsyncMaterialCache::create(std::shared_ptr<MaterialConfig> keyObj)
{
    auto newMat = std::make_unique<Material>(runningId++, keyObj, pipelineCache->getPipeline(keyObj->getPipeline()));

    auto& tConfigs = keyObj->getBindingConfigs();
    for (auto& entry : tConfigs) {
        auto& c = entry.second;
        // fine to join. this func runs in the task so it will block the task's thread
        auto binding = c->getBinding(*textureCache, *bufferCache).get();
        newMat->addBinding(std::move(binding));
    }

    return newMat;
}
