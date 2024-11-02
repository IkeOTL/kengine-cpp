#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/mesh/Vertex.hpp>
#include <vector>
#include <memory>

namespace ke {
    template<typename V>
    class MeshBuilder {
    private:
        static_assert(std::is_base_of<Vertex, V>::value, "V must be derived from Vertex");

        const int vertexAttributes;

        std::vector<uint32_t> indices;
        std::vector<V> vertices;

    public:
        MeshBuilder(int vertexAttributes)
            : vertexAttributes(vertexAttributes) {}

        /// <summary>
        /// will NOT add/init verts
        /// </summary>
        void reserve(uint32_t indexCnt, uint32_t vertCnt) {
            indices.reserve(indexCnt);
            vertices.reserve(vertCnt);
        }

        /// <summary>
        /// WILL add/init verts
        /// </summary>
        void resize(uint32_t indexCnt, uint32_t vertCnt) {
            indices.resize(indexCnt);
            vertices.resize(vertCnt);
        }

        std::vector<uint32_t>& getIndices() {
            return indices;
        }

        std::vector<V>& getVertices() {
            return vertices;
        }

        uint32_t getIndexCount() {
            return indices.size();
        }

        uint32_t getVertexCount() const {
            return vertices.size();
        }

        int getVertexAttributes() const {
            return vertexAttributes;
        }

        V& getVertex(uint32_t i) const {
            return vertices[i];
        }

        std::unique_ptr<Mesh> build(VulkanContext* vkContext) {
            return build(vkContext, false, false);
        }

        V createVertex() const {
            return V();
        }

        uint32_t pushVertex(V&& vert) {
            vertices.push_back(std::move(vert));
            return vertices.size() - 1;
        }

        void pushIndex(uint32_t i1) {
            indices.push_back(i1);
        }

        void pushTriangle(uint32_t i1, uint32_t i2, uint32_t i3) {
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }

        std::unique_ptr<Mesh> build(VulkanContext* vkContext, bool generateNormals, bool generateTangents) {
            if constexpr (std::is_base_of_v<TexturedVertex, V>) {
                if (generateNormals)
                    calculateNormals();

                if (generateTangents)
                    calculateTangents();
            }

            // used in systems that dont render, like servers
            if (!vkContext) {
                // todo: no need for vertdata, just calcuate bounds without it
                auto vertData = std::make_unique<VertexData<V>>(std::move(vertices), getVertexAttributes(), getIndexCount(), getVertexCount());
                vertData->calcBounds();
                return std::make_unique<Mesh>(
                    getVertexAttributes(),
                    getIndexCount(), nullptr,
                    getVertexCount(), nullptr,
                    vertData->getBounds());
            }

            auto xferFlag = 0;

            auto idxBuffer = vkContext->uploadBuffer(
                [&lol = indices](VulkanContext& vkCxt, void* data) {
                    memcpy(data, lol.data(), lol.size() * sizeof(uint32_t));
                },
                indices.size() * sizeof(uint32_t),
                VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
                VK_ACCESS_2_INDEX_READ_BIT,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                0,
                nullptr);

            auto vertBuffer = vkContext->uploadBuffer(
                [&lol = vertices](VulkanContext& vkCxt, void* data) {
                    memcpy(data, lol.data(), V::sizeOf() * lol.size());
                },
                V::sizeOf() * vertices.size(),
                VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
                VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                0,
                nullptr);

            // todo: no need for vertdata, just calcuate bounds without it
            auto vertData = std::make_unique<VertexData<V>>(std::move(vertices), getVertexAttributes(), getIndexCount(), getVertexCount());
            vertData->calcBounds();

            return std::make_unique<Mesh>(
                getVertexAttributes(),
                getIndexCount(), std::move(idxBuffer),
                getVertexCount(), std::move(vertBuffer),
                vertData->getBounds());
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

                p1 = vertices[index1].position;
                p2 = vertices[index2].position;
                p3 = vertices[index3].position;

                u = p2 - p1;
                v = p3 - p1;
                u = glm::cross(u, v);

                // need to confirm this updates the ref in the array
                vertices[index1].normal += u;
                vertices[index2].normal += u;
                vertices[index3].normal += u;
            }

            for (auto& vert : vertices)
                vert.normal = glm::normalize(vert.normal);
        }

        void calculateTangents() {
            glm::vec4 tangent{};
            glm::vec3 edge1{};
            glm::vec3 edge2{};
            for (auto i = 0; i < indices.size(); i += 3) {
                auto i0 = indices[i];
                auto i1 = indices[i + 1];
                auto i2 = indices[i + 2];

                edge1 = vertices[i1].position - vertices[i0].position;
                edge2 = vertices[i2].position - vertices[i0].position;

                auto deltaU1 = vertices[i1].texCoords.x - vertices[i0].texCoords.x;
                auto deltaV1 = vertices[i1].texCoords.y - vertices[i0].texCoords.y;
                auto deltaU2 = vertices[i2].texCoords.x - vertices[i0].texCoords.x;
                auto deltaV2 = vertices[i2].texCoords.y - vertices[i0].texCoords.y;

                auto dividend = deltaU1 * deltaV2 - deltaU2 * deltaV1;

                auto f = dividend == 0 ? 0.0f : 1.0f / dividend;

                tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
                tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
                tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

                // need to confirm this updates the ref in the array
                vertices[i0].tangent += tangent;
                vertices[i1].tangent += tangent;
                vertices[i2].tangent += tangent;
            }

            for (auto& vert : vertices)
                vert.tangent = glm::normalize(vert.tangent);
        }
    };
} // namespace ke