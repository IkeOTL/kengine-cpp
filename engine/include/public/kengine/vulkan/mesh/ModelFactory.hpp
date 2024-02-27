#pragma once
#include <kengine/vulkan/mesh/Model.hpp>
#include <string>
#include <memory>

class ModelFactory {
public:
    virtual std::unique_ptr<Model> loadModel(std::string meshKey, int vertexAttributes) = 0;
};