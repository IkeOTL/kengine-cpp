#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <kengine/vulkan/material/MaterialBinding.hpp>
#include <kengine/vulkan/pipelines/Pipeline.hpp>
#include <mutex>

class Material {

private:
    const int id;
    std::shared_ptr<MaterialConfig> config;

    Pipeline& pipeline;
    std::unordered_map<int, std::vector<std::unique_ptr<MaterialBinding>>> materialBindings;

public:
    Material(const int id, std::shared_ptr<MaterialConfig> config, Pipeline& pipeline)
        : id(id), config(config), pipeline(pipeline) {}

    int getId() {
        return id - AsyncMaterialCache::START_ID;
    }

    MaterialBinding& getBinding(int descSetIdx, int bindingIdx);

    void addBinding(std::unique_ptr<MaterialBinding>&& binding);
};