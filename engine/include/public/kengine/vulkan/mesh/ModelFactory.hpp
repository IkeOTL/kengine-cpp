#pragma once
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/vulkan/mesh/ModelConfig.hpp>
#include <string>
#include <memory>

namespace ke {
    class ModelFactory {
    public:
        virtual std::unique_ptr<Model> loadModel(const ModelConfig& config) = 0;
    };
} // namespace ke