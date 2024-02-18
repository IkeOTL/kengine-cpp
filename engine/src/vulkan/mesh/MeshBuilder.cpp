#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>

void IndexBuffer::upload(VulkanContext& vkCxt, void* data) {
    memcpy(data, indices.data(), size());
}

VkDeviceSize IndexBuffer::size() {
    return sizeof(uint32_t) * indices.size();
}

void VertexBuffer::upload(VulkanContext& vkCxt, void* data) {
    memcpy(data, vertices.data(), size());
}

VkDeviceSize VertexBuffer::size() {
    // assume entire vector is of same vertex type
    return vertices[0]->sizeOf() * vertices.size();
}

std::unique_ptr<Vertex> MeshBuilder::createVertex(int vertexAttributes) {
    if (vertexAttributes & VertexAttribute::TEX_COORDS)
        if (vertexAttributes & VertexAttribute::SKELETON)
            return std::unique_ptr<RiggedTexturedVertex>();
        else if (vertexAttributes & VertexAttribute::NORMAL)
            return std::unique_ptr<TexturedVertex>();
        else
            return std::unique_ptr<SimpleTexturedVertex>();
    else if (vertexAttributes & VertexAttribute::COLOR)
        if (vertexAttributes & VertexAttribute::SKELETON)
            return std::unique_ptr<RiggedColoredVertex>();
        else if (vertexAttributes & VertexAttribute::NORMAL)
            return std::unique_ptr<ColoredVertex>();
        else
            return std::unique_ptr<SimpleColoredVertex>();

    return std::unique_ptr<Vertex>();
}

uint32_t MeshBuilder::pushVertex(std::unique_ptr<Vertex>&& vert) {
    vertices.push_back(std::move(vert));
    return vertices.size() - 1;
}

void MeshBuilder::pushIndex(int i1) {
    indices.push_back(i1);
}

void MeshBuilder::pushTriangle(int i1, int i2, int i3) {
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}


std::unique_ptr<Mesh> MeshBuilder::build(VulkanContext* vkContext, bool generateNormals, bool generateTangents) {
    if (generateNormals)
        calculateNormals();

    if (generateTangents)
        calculateTangents();

    // used in systems that dont render, like servers
    if (!vkContext) {
        auto meshData = std::make_unique<MeshData>(std::move(vertices), getIndexCount(), getVertexCount());
        return std::make_unique<Mesh>(std::move(meshData), nullptr, nullptr);
    }

    auto xferFlag = 0;

    auto idxBuffer = std::make_unique<IndexBuffer>(indices);
    vkContext->uploadBuffer(*idxBuffer,
        VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_ACCESS_2_INDEX_READ_BIT,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, xferFlag, nullptr);

    auto vertBuffer = std::make_unique<VertexBuffer>(getVertexAttributes(), vertices);
    vkContext->uploadBuffer(*vertBuffer,
        VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, xferFlag, nullptr);

    auto meshData = std::make_unique<MeshData>(std::move(vertices), getVertexAttributes(), getIndexCount(), getVertexCount());
    //meshData->calcAabb();
    return std::make_unique<Mesh>(std::move(meshData), std::move(idxBuffer->releaseBuffer()), std::move(vertBuffer->releaseBuffer()));
}


void MeshBuilder::calculateNormals() {
    Vector3f p1 = new Vector3f();
    Vector3f p2 = new Vector3f();
    Vector3f p3 = new Vector3f();
    Vector3f u = new Vector3f();
    Vector3f v = new Vector3f();

    for (int i = 0; i < indices.size(); i += 3) {
        int index1 = indices.get(i);
        int index2 = indices.get(i + 1);
        int index3 = indices.get(i + 2);

        p1.set(vertices.get(index1).getPosition());
        p2.set(vertices.get(index2).getPosition());
        p3.set(vertices.get(index3).getPosition());

        u.set(p2).sub(p1);
        v.set(p3).sub(p1);
        u.cross(v);

        vertices.get(index1).getNormal().add(u);
        vertices.get(index2).getNormal().add(u);
        vertices.get(index3).getNormal().add(u);
    }

    for (var verts : vertices) {
        verts.getNormal().normalize();
    }
}

void MeshBuilder::calculateTangents() {
    var tangent = new Vector4f(0, 0, 0, 0);
    var edge1 = new Vector3f();
    var edge2 = new Vector3f();
    for (var i = 0; i < indices.size(); i += 3) {
        var i0 = indices.get(i);
        var i1 = indices.get(i + 1);
        var i2 = indices.get(i + 2);

        edge1.set(vertices.get(i1).getPosition()).sub(vertices.get(i0).getPosition());
        edge2.set(vertices.get(i2).getPosition()).sub(vertices.get(i0).getPosition());

        var deltaU1 = vertices.get(i1).getTexCoords().x - vertices.get(i0).getTexCoords().x;
        var deltaV1 = vertices.get(i1).getTexCoords().y - vertices.get(i0).getTexCoords().y;
        var deltaU2 = vertices.get(i2).getTexCoords().x - vertices.get(i0).getTexCoords().x;
        var deltaV2 = vertices.get(i2).getTexCoords().y - vertices.get(i0).getTexCoords().y;

        var dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);

        var f = dividend == 0 ? 0.0f : 1.0f / dividend;

        tangent.set(
            f * (deltaV2 * edge1.x - deltaV1 * edge2.x),
            f * (deltaV2 * edge1.y - deltaV1 * edge2.y),
            f * (deltaV2 * edge1.z - deltaV1 * edge2.z)
        );

        vertices.get(i0).getTangent().add(tangent);
        vertices.get(i1).getTangent().add(tangent);
        vertices.get(i2).getTangent().add(tangent);
    }

    for (var vertices1 : vertices) {
        vertices1.getTangent().normalize();
    }
}