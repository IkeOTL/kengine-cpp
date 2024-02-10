#include <kengine/vulkan/mesh/Mesh.hpp>

const std::vector<std::unique_ptr<Vertex>>& MeshData::getVertices() const {
    return vertices;
}

const std::vector<std::unique_ptr<Vertex>>& Mesh::getVertices() const {
    return meshData->getVertices();
}

uint32_t Mesh::getIndexCount() const {
    return meshData->getIndexCount();
}

uint32_t Mesh::getVertexCount() const {
    return meshData->getVertexCount();
}

const GpuBuffer& Mesh::getIndexBuf() const {
    return *indexBuf;
}

const GpuBuffer& Mesh::getVertexBuf() const {
    return *vertexBuf;
}