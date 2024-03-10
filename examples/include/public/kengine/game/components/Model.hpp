#pragma once
#include <kengine/vulkan/mesh/ModelConfig.hpp>
#include <memory>

namespace Component {
    struct Model {
        std::shared_ptr<ModelConfig> config;
    };
}