#pragma once
#include <kengine/vulkan/material/MaterialConfig.hpp>
#include <memory>

namespace Component {
    struct Material {
        std::shared_ptr<MaterialConfig> config;
    };
}