#pragma once
#include <kengine/Hashable.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <string>
#include <memory>

class ModelConfig : Hashable {
private:
    std::string modelKey;
    int32_t attributes;

public:
    ModelConfig(std::string modelKey, int32_t attributes)
        : modelKey(modelKey), attributes(attributes) {}

    ModelConfig(std::string modelKey)
        : ModelConfig(modelKey, VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS) {}

    inline static std::shared_ptr<ModelConfig> create(std::string modelKey, int32_t attributes) {
        return std::make_shared<ModelConfig>(modelKey, attributes);
    }

    inline static std::shared_ptr<ModelConfig> create(std::string modelKey) {
        return std::make_shared<ModelConfig>(modelKey, VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::TEX_COORDS);
    }

    std::string getModelKey() const {
        return modelKey;
    }

    int32_t getAttributes() const {
        return attributes;
    }

    size_t hashCode() const noexcept override;
    bool operator==(const ModelConfig& other) const;
    bool operator!=(const ModelConfig& other) const;
};