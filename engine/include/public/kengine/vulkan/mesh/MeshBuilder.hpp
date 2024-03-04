#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <vector>
#include <memory>

template <typename V>
class MeshBuilder;

class IndexBuffer : public GpuUploadable {
private:
    std::vector<uint16_t>& indices;

public:
    IndexBuffer(std::vector<uint16_t>& indices) : indices(indices) {}

    void upload(VulkanContext& vkCxt, void* data) override {
        memcpy(data, indices.data(), size());
    }

    VkDeviceSize size() override {
        return sizeof(uint16_t) * indices.size();
    }
};

template <typename V>
class VertexBuffer : public GpuUploadable {
private:
    static_assert(std::is_base_of<Vertex, V>::value, "V must be derived from Vertex");

    std::vector<V>& vertices;

public:
    VertexBuffer(std::vector<V>& vertices) : vertices(vertices) {}

    void upload(VulkanContext& vkCxt, void* data) override {
        memcpy(data, vertices.data(), size());
    }

    VkDeviceSize VertexBuffer::size() override {
        return vertices[0]->sizeOf() * vertices.size();
    }
};

template <typename V>
class MeshBuilder {
private:
    static_assert(std::is_base_of<Vertex, V>::value, "V must be derived from Vertex");

    const int vertexAttributes;

    std::vector<uint16_t> indices;
    std::vector<V> vertices;

    void calculateNormals();
    void calculateTangents();

public:
    MeshBuilder(int vertexAttributes) : vertexAttributes(vertexAttributes) {}

    /// <summary>
    /// will NOT add/init verts
    /// </summary>
    void reserve(uint16_t indexCnt, uint16_t vertCnt) {
        indices.reserve(indexCnt);
        vertices.reserve(vertCnt);
    }

    /// <summary>
    /// WILL add/init verts
    /// </summary>
    void resize(uint16_t indexCnt, uint16_t vertCnt) {
        indices.resize(indexCnt);
        vertices.resize(vertCnt);
    }

    std::vector<uint16_t>& getIndices() const {
        return indices;
    }

    std::vector<V>& getVertices() const {
        return vertices;
    }

    uint16_t getIndexCount() {
        return indices.size();
    }

    uint16_t getVertexCount() const {
        return vertices.size();
    }

    int getVertexAttributes() const {
        return vertexAttributes;
    }

    V& getVertex(uint16_t i) const {
        return vertices[i];
    }

    std::unique_ptr<Mesh> build(VulkanContext* vkContext) {
        return build(vkContext, false, false);
    }

    std::unique_ptr<Mesh> build(VulkanContext* vkContext, bool generateNormals, bool generateTangents);

    V createVertex() const {
        return V();
    }

    uint16_t pushVertex(V&& vert) {
        vertices.push_back(std::move(vert));
        return vertices.size() - 1;
    }

    void pushIndex(uint16_t i1) {
        indices.push_back(i1);
    }

    void pushTriangle(uint16_t i1, uint16_t i2, uint16_t i3) {
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i3);
    }

    std::unique_ptr<Mesh> build(VulkanContext* vkContext, bool generateNormals, bool generateTangents) {
        if (generateNormals)
            calculateNormals();

        if (generateTangents)
            calculateTangents();

        // used in systems that dont render, like servers
        if (!vkContext) {
            auto meshData = std::make_unique<MeshData>(std::move(vertices), getVertexAttributes(), getIndexCount(), getVertexCount());
            return std::make_unique<Mesh>(std::move(meshData), nullptr, nullptr);
        }

        auto xferFlag = 0;

        auto idxBuffer = std::make_unique<IndexBuffer>(indices);
        vkContext->uploadBuffer(*idxBuffer,
            VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_ACCESS_2_INDEX_READ_BIT,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT, xferFlag, nullptr);

        auto vertBuffer = std::make_unique<VertexBuffer>(vertices);
        vkContext->uploadBuffer(*vertBuffer,
            VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, xferFlag, nullptr);

        auto meshData = std::make_unique<MeshData>(std::move(vertices), getVertexAttributes(), getIndexCount(), getVertexCount());
        meshData->calcBounds();
        return std::make_unique<Mesh>(std::move(meshData), std::move(idxBuffer->releaseBuffer()), std::move(vertBuffer->releaseBuffer()));
    }

private:
    void calculateNormals() {
        glm::vec3 p1{};
        glm::vec3 p2{};
        glm::vec3 p3{};
        glm::vec3 u{};
        glm::vec3 v{};

        for (auto i = 0; i < indices.size(); i += 3) {
            auto index1 = indices[i];
            auto index2 = indices[i + 1];
            auto index3 = indices[i + 2];

            p1 = vertices[index1]->getPosition();
            p2 = vertices[index2]->getPosition();
            p3 = vertices[index3]->getPosition();

            u = p2 - p1;
            v = p3 - p1;
            u = glm::cross(u, v);

            *vertices[index1]->getNormal() += u;
            *vertices[index2]->getNormal() += u;
            *vertices[index3]->getNormal() += u;
        }

        for (auto& vert : vertices)
            vert->setNormal(glm::normalize(*vert->getNormal()));
    }

    void calculateTangents() {
        glm::vec4 tangent{};
        glm::vec3 edge1{};
        glm::vec3 edge2{};
        for (auto i = 0; i < indices.size(); i += 3) {
            auto i0 = indices[i];
            auto i1 = indices[i + 1];
            auto i2 = indices[i + 2];


            edge1 = vertices[i1]->getPosition() - vertices[i0]->getPosition();
            edge2 = vertices[i2]->getPosition() - vertices[i0]->getPosition();

            auto deltaU1 = vertices[i1]->getTexCoords()->x - vertices[i0]->getTexCoords()->x;
            auto deltaV1 = vertices[i1]->getTexCoords()->y - vertices[i0]->getTexCoords()->y;
            auto deltaU2 = vertices[i2]->getTexCoords()->x - vertices[i0]->getTexCoords()->x;
            auto deltaV2 = vertices[i2]->getTexCoords()->y - vertices[i0]->getTexCoords()->y;

            auto dividend = deltaU1 * deltaV2 - deltaU2 * deltaV1;

            auto f = dividend == 0 ? 0.0f : 1.0f / dividend;

            tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
            tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
            tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

            *vertices[i0]->getTangent() += tangent;
            *vertices[i1]->getTangent() += tangent;
            *vertices[i2]->getTangent() += tangent;
        }

        for (auto& vert : vertices)
            vert->setTangent(glm::normalize(*vert->getTangent()));
    }
};