#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/material/AsyncMaterialCache.hpp>
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <mutex>

class Material {

private:
    const int id;
    MaterialConfig config;

    Pipeline& pipeline;
    std::unordered_map<int, std::vector<std::unique_ptr<MaterialBinding>>> materialBindings;

public:
    Material(int id, MaterialConfig config, Pipeline& pipeline)
        : id(id), config(config), pipeline(pipeline) {}

    int getId() {
        return id - AsyncMaterialCache::START_ID;
    }

    MaterialBinding& getBinding(int descSetIdx, int bindingIdx) {
        auto it = materialBindings.find(descSetIdx);

        if (it == materialBindings.end())
            throw std::runtime_error("Material binding not found.");

        return *(it->second[bindingIdx]);
    }
};