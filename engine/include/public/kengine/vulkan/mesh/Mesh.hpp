#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>

#include <vector>
#include <memory>
#include <kengine/Bounds.hpp>

class MeshData {
private:
    const int vertexAttributes;

    const int indexCount;
    const int vertexCount;

    const std::vector<std::unique_ptr<Vertex>> vertices;

    Bounds bounds{};

public:
    MeshData(std::vector<std::unique_ptr<Vertex>>&& vertices, int vertexAttributes, uint32_t indexCount, uint32_t vertexCount)
        : vertices(std::move(vertices)), vertexAttributes(vertexAttributes),
        indexCount(indexCount), vertexCount(vertexCount) {}

    int getVertexAttributes() const {
        return vertexAttributes;
    }

    const std::vector<std::unique_ptr<Vertex>>& getVertices() const;

    const Bounds& getBounds() const {
        return bounds;
    }

    void calcBounds();

    int getIndexCount() const {
        return indexCount;
    }

    int getVertexCount() const {
        return vertexCount;
    }
};

/// <summary>
/// The geometry thats uploaded to the GPU
/// </summary>
class Mesh {
private:
    const std::unique_ptr<MeshData> meshData;

    const std::unique_ptr<GpuBuffer> indexBuf;
    const std::unique_ptr<GpuBuffer> vertexBuf;

public:
    Mesh(std::unique_ptr<MeshData>&& meshData, std::unique_ptr<GpuBuffer>&& indexBuf, std::unique_ptr<GpuBuffer>&& vertexBuf)
        : meshData(std::move(meshData)), indexBuf(std::move(indexBuf)), vertexBuf(std::move(vertexBuf)) {}

    const std::vector<std::unique_ptr<Vertex>>& getVertices() const;

    const MeshData& getMeshData() const {
        return *meshData;
    }

    uint32_t getIndexCount() const;
    uint32_t getVertexCount() const;

    const GpuBuffer& getIndexBuf() const;
    const GpuBuffer& getVertexBuf() const;
};