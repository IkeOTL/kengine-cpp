#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>

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
    // maybe create a funiction that accounts for vertex attributes 
    // incase this is ever completely empty
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
    //meshData->calcAabb();
    return std::make_unique<Mesh>(std::move(meshData), std::move(idxBuffer->releaseBuffer()), std::move(vertBuffer->releaseBuffer()));
}


void MeshBuilder::calculateNormals() {
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

void MeshBuilder::calculateTangents() {
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