#pragma once
#include <kengine/vulkan/mesh/ModelFactory.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/io/AssetIO.hpp>
#include <thirdparty/tiny_gltf.h>
#include <unordered_set>
#include <thread>

class VulkanContext;
class assetIo;

class GltfModelFactory : public ModelFactory {
private:
    static thread_local tinygltf::TinyGLTF gltfLoader;

    VulkanContext& vkContext;
    AssetIO& assetIo;

public:
    GltfModelFactory(VulkanContext& vkContext, AssetIO& assetIo)
        : vkContext(vkContext), assetIo(assetIo) {}

    std::unique_ptr<Model> loadModel(std::string meshKey, int vertexAttributes) override;

private:
    void processNode(const tinygltf::Model& model, int nodeIndex, std::unordered_set<int>& meshIndices) const;
    void loadMeshGroup(const tinygltf::Model& model, int meshGroupIdx, std::unordered_map<int, std::unique_ptr<MeshGroup>>& mesheGroups, int vertexAttributes) const;

    template <typename T>
    const unsigned char* getAttrBuffer(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string attributeName, size_t& count, size_t& stride) const {
        auto it = primitive.attributes.find(attributeName);
        if (it == primitive.attributes.end())
            throw std::runtime_error("Vertex attribute not found.");

        auto accessorIndex = it->second;
        const auto& accessor = model.accessors[accessorIndex];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];
        const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;

        count = accessor.count;
        stride = bufferView.byteStride ? bufferView.byteStride : sizeof(T);
        return&buffer.data[byteOffset];
    }

    template <typename V>
    void loadMesh(const tinygltf::Model& model, MeshBuilder<V>& mb, const tinygltf::Primitive& primitive, MeshGroup& meshGroup) const {
        // load indices
        {
            auto indicesIdx = primitive.indices;

            if (indicesIdx == -1)
                throw std::runtime_error("Currently only support indexed meshes.");

            const auto& accessor = model.accessors[indicesIdx];

            if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                && accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                throw std::runtime_error("Component type not handled.");

            // resize mesh builder
            {
                const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                mb.resize(accessor.count, posAccessor.count);
            }

            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];

            // Accessor could define a byte offset within the bufferView
            const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
            const auto meshIndices = &(buffer.data[byteOffset]);

            auto& targetIndices = mb.getIndices();
            // assume builder has had resize() executed
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const auto meshIndices = reinterpret_cast<const uint16_t*>(&(buffer.data[byteOffset]));
                for (auto i = 0; i < accessor.count; i++)
                    targetIndices[i] = meshIndices[i];
            }
            else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                memcpy(targetIndices.data(), &(buffer.data[byteOffset]), accessor.count * sizeof(uint32_t));
        }

        // load vertices
        {
            auto& verts = mb.getVertices();

            auto vertexAttributes = mb.getVertexAttributes();
            if (vertexAttributes & VertexAttribute::POSITION) {
                size_t count;
                size_t stride;
                const auto* attr = getAttrBuffer<glm::vec3>(model, primitive, "POSITION", count, stride);
                for (auto i = 0; i < count; i++)
                    memcpy(&(verts[i].position), attr + (i * stride), sizeof(glm::vec3));
            }

            if (vertexAttributes & VertexAttribute::NORMAL) {
                if constexpr (std::is_base_of_v<V, TexturedVertex> || std::is_base_of_v<V, ColoredVertex>) {
                    size_t count;
                    size_t stride;
                    const auto* attr = getAttrBuffer<glm::vec3>(model, primitive, "NORMAL", count, stride);
                    for (auto i = 0; i < count; i++)
                        memcpy(&(verts[i].normal), attr + (i * stride), sizeof(glm::vec3));
                }
            }

            if (vertexAttributes & VertexAttribute::COLOR) {
                // not used atm
            }

            if (vertexAttributes & VertexAttribute::TEX_COORDS) {
                if constexpr (std::is_base_of_v<V, TexturedVertex>) {
                    size_t count;
                    size_t stride;
                    const auto attr = getAttrBuffer<glm::vec2>(model, primitive, "TEXCOORD_0", count, stride);
                    for (auto i = 0; i < count; i++)
                        memcpy(&(verts[i].texCoords), attr + (i * stride), sizeof(glm::vec2));
                }
            }

            if (vertexAttributes & VertexAttribute::TANGENTS) {
                if constexpr (std::is_base_of_v<V, TexturedVertex> || std::is_base_of_v<V, ColoredVertex>) {
                    size_t count;
                    size_t stride;
                    const auto attr = getAttrBuffer(model, primitive, "TANGENT", count, stride);
                    for (auto i = 0; i < count; i++)
                        memcpy(&(verts[i].tangent), attr + (i * stride), sizeof(glm::vec4));
                }
            }

            if (vertexAttributes & VertexAttribute::SKELETON) {
                if constexpr (std::is_base_of_v<V, RiggedTexturedVertex> || std::is_base_of_v<V, RiggedColoredVertex>) {
                    // joints
                    {
                        size_t count;
                        size_t stride;
                        const auto attr = getAttrBuffer(model, primitive, "JOINTS_0", count, stride);

                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            for (auto i = 0; i < count; i++)
                                memcpy(&(verts[i].blendIndex), attr + (i * stride), sizeof(glm::uvec4));
                        }
                    }
                }
            }
        }

        meshGroup.addMesh(
            mb.build(&vkContext
                //,
                //vertexAttributes | VertexAttribute::NORMAL,
                //vertexAttributes | VertexAttribute::TANGENTS
            )
        );
    }

};