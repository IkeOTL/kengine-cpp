#pragma once
#include <vector>
#include <memory>

class VulkanContext;
class Mesh;
class Vertex;

class IndexBuffer : public GpuUploadable {
    std::vector<uint32_t>& indices;

    IndexBuffer(std::vector<uint32_t>& indices) :indices(indices) {}

    void upload(VulkanContext& vkCxt, void* data) override;
    VkDeviceSize size() override;
};

class VertexBuffer : public GpuUploadable {
    std::vector<std::unique_ptr<Vertex>>& vertices;

    VertexBuffer(std::vector<std::unique_ptr<Vertex>>& vertices) : vertices(vertices) {}

    void upload(VulkanContext& vkCxt, void* data) override;
    VkDeviceSize size() override;
};

class MeshBuilder {
private:
    int vertexAttributes;

    std::vector<uint32_t> indices;
    std::vector<std::unique_ptr<Vertex>> vertices;

public:
    MeshBuilder(int vertexAttributes) : vertexAttributes(vertexAttributes) {}

    std::unique_ptr<Vertex> createVertex(int vertexAttributes);
    uint32_t pushVertex(std::unique_ptr<Vertex>&& vert);
    void pushIndex(int i1);
    void pushTriangle(int i1, int i2, int i3);


    uint32_t getIndexCount() {
        return indices.size();
    }

    uint32_t getVertexCount() {
        return vertices.size();
    }

    int getVertexAttributes() {
        return vertexAttributes;
    }

    Vertex& getVertex(uint32_t i) {
        return *vertices[i];
    }

    std::unique_ptr<Mesh> build(VulkanContext* vkContext) {
        return build(vkContext, false, false);
    }

    std::unique_ptr<Mesh> build(VulkanContext* vkContext, bool generateNormals, bool generateTangents);

private:
    void calculateNormals();
    void calculateTangents();
};