#pragma once
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <vector>

class MeshData {
private:
    const int vertexAttributes;

    const int indexCount;
    const int vertexCount;

    const std::vector<std::unique_ptr<Vertex>> vertices;

public:
    MeshData(std::vector<std::unique_ptr<Vertex>>&& vertices, int vertexAttributes, int indexCount, int vertexCount)
        : vertices(std::move(vertices)), vertexAttributes(vertexAttributes),
        indexCount(indexCount), vertexCount(vertexCount) {}

    int getVertexAttributes() const {
        return vertexAttributes;
    }

    const std::vector<Vertex>& getVertices() const {
        return vertices;
    }

    int getIndexCount() const {
        return indexCount;
    }

    int getVertexCount() const {
        return vertexCount;
    }
};