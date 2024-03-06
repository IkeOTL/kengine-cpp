#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <kengine/Bounds.hpp>

#include <glm/glm.hpp>
#include <vector>
#include <memory>

template <typename V>
class VertexData {
private:
    static_assert(std::is_base_of<Vertex, V>::value, "V must be derived from Vertex");

    const std::vector<V> vertices;

    const int vertexAttributes;
    const int indexCount;
    const int vertexCount;

    Bounds bounds{};

public:
    VertexData(std::vector<V>&& vertices, int vertexAttributes, uint32_t indexCount, uint32_t vertexCount)
        : vertices(std::move(vertices)), vertexAttributes(vertexAttributes),
        indexCount(indexCount), vertexCount(vertexCount) {}

    int getVertexAttributes() const {
        return vertexAttributes;
    }

    const std::vector<V>& getVertices() const {
        return vertices;
    }

    const Bounds& getBounds() const {
        return bounds;
    }

    int getIndexCount() const {
        return indexCount;
    }

    int getVertexCount() const {
        return vertexCount;
    }

    void calcBounds() {
        glm::vec3 _min(9999999);
        glm::vec3 _max(-9999999);

        for (const V& vert : vertices) {
            const auto& pos = vert.position;
            _min = glm::min(pos, _min);
            _max = glm::max(pos, _max);
        }

        bounds = Bounds::fromMinMax(_min, _max);
    }
};

class Mesh {
private:
    const int vertexAttributes;

    const std::unique_ptr<GpuBuffer> indexBuf;
    const std::unique_ptr<GpuBuffer> vertexBuf;

    const uint32_t indexCount;
    const uint32_t vertexCount;

    const Bounds bounds;

public:
    Mesh(int vertexAttributes, uint32_t indexCount, std::unique_ptr<GpuBuffer>&& indexBuf,
        uint32_t vertexCount, std::unique_ptr<GpuBuffer>&& vertexBuf,
        Bounds bounds)
        : vertexAttributes(vertexAttributes),
        indexCount(indexCount), indexBuf(std::move(indexBuf)),
        vertexCount(vertexCount), vertexBuf(std::move(vertexBuf)),
        bounds(bounds) {}

    ~Mesh() = default;

    const GpuBuffer& getIndexBuf() const {
        return *indexBuf;
    }

    const GpuBuffer& getVertexBuf() {
        return *vertexBuf;
    }

    const Bounds& getBounds() const {
        return bounds;
    }

    uint32_t getIndexCount() const {
        return indexCount;
    }

    uint32_t getVertexCount() const {
        return vertexCount;
    }
};