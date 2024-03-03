#pragma once
#include <kengine/vulkan/mesh/ModelFactory.hpp>


class GltfModelFactory : public ModelFactory {
    std::unique_ptr<Model> loadModel(std::string meshKey, int vertexAttributes) override;
};