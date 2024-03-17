#pragma once
#include <kengine/vulkan/mesh/ModelConfig.hpp>
#include <memory>

namespace Component {
    struct ModelComponent {
        std::shared_ptr<ModelConfig> config;
    };
}