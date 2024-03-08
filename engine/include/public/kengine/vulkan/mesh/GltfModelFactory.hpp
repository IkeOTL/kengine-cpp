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

    const tinygltf::Accessor& getAccessor(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string attributeName) {
        auto it = primitive.attributes.find(attributeName);
        if (it == primitive.attributes.end())
            throw std::runtime_error("Vertex attribute not found.");

        auto accessorIndex = it->second;
        return model.accessors[accessorIndex];
    }

    template <typename T>
    const unsigned char* getAttrBuffer(const tinygltf::Model& model, const tinygltf::Accessor& accessor, size_t& count, size_t& stride) const {
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];
        const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;

        count = accessor.count;
        stride = bufferView.byteStride ? bufferView.byteStride : sizeof(T);
        return &buffer.data[byteOffset];
    }

    template <typename T>
    const unsigned char* getAttrBuffer(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string attributeName, size_t& count, size_t& stride) const {
        const auto& accessor = getAccessor(model, primitive, attributeName);
        return getAttrBuffer(model, accessor, count, stride);
    }

    template <typename V>
    void loadMesh(const tinygltf::Model& model, MeshBuilder<V>& mb, const tinygltf::Primitive& primitive, MeshGroup& meshGroup) const {
        // load verts
        const unsigned char* positionAttr = nullptr;
        const unsigned char* normalAttr = nullptr;
        const unsigned char* colorAttr = nullptr;
        const unsigned char* texCoordAttr = nullptr;
        const unsigned char* tangentAttr = nullptr;
        const unsigned char* jointIndexAttr = nullptr;
        const unsigned char* jointWeightAttr = nullptr;

        size_t vertCount = 0;
        size_t positionStride = 0, normalStride = 0, colorStride = 0, texCoordStride = 0,
            tangentStride = 0, jointIndexStride = 0, jointWeightStride = 0;
        int jointComponentType;

        if (vertexAttributes & VertexAttribute::POSITION)
            positionAttr = getAttrBuffer<glm::vec3>(model, primitive, "POSITION", vertCount, positionStride);

        if (vertexAttributes & VertexAttribute::NORMAL)
            normalAttr = getAttrBuffer<glm::vec3>(model, primitive, "NORMAL", vertCount, normalStride);

        if (vertexAttributes & VertexAttribute::COLOR) { /* probably wont even use */ }

        if (vertexAttributes & VertexAttribute::TEX_COORDS)
            texCoordAttr = getAttrBuffer<glm::vec2>(model, primitive, "TEXCOORD_0", vertCount, texCoordStride);

        if (vertexAttributes & VertexAttribute::TANGENTS)
            tangentAttr = getAttrBuffer<glm::vec4>(model, primitive, "TANGENT", vertCount, tangentStride);

        if (vertexAttributes & VertexAttribute::SKELETON) {
            // use accessor directly, since joints could be either byte or short
            const auto& accessor = getAccessor(model, primitive, "JOINTS_0");
            jointComponentType = accessor.componentType;
            if (jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                jointIndexAttr = getAttrBuffer<glm::u8vec4>(model, accessor, vertCount, jointIndexStride);
            else if (jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                jointIndexAttr = getAttrBuffer<glm::u16vec4>(model, accessor, vertCount, jointIndexStride);

            jointWeightAttr = getAttrBuffer<glm::vec4>(model, primitive, "WEIGHTS_0", vertCount, jointWeightStride);
        }

        auto& verts = mb.getVertices();
        for (size_t i = 0; i < vertCount; i++) {
            memcpy(&(verts[i].position), positionAttr + (i * positionStride), sizeof(glm::vec3));

            if constexpr (std::is_base_of_v<V, TexturedVertex> || std::is_base_of_v<V, ColoredVertex>)
                memcpy(&(verts[i].normal), normalAttr + (i * normalStride), sizeof(glm::vec3));

            if constexpr (std::is_base_of_v<V, ColoredVertex>) { /* probably wont even use */ }

            if constexpr (std::is_base_of_v<V, TexturedVertex>)
                memcpy(&(verts[i].texCoords), texCoordAttr + (i * texCoordStride), sizeof(glm::vec2));

            if constexpr (std::is_base_of_v<V, TexturedVertex> || std::is_base_of_v<V, ColoredVertex>)
                memcpy(&(verts[i].tangents), tangentAttr + (i * tangentStride), sizeof(glm::vec4));

            if constexpr (std::is_base_of_v<V, RiggedTexturedVertex> || std::is_base_of_v<V, RiggedColoredVertex>) {
                if (jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    memcpy(&(verts[i].blendIndex), jointIndexAttr + (i * jointIndexStride), sizeof(glm::u8vec4));
                else if (jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    memcpy(&(verts[i].blendIndex), jointIndexAttr + (i * jointIndexStride), sizeof(glm::u16vec4));

                memcpy(&(verts[i].blendWeight), jointWeightAttr + (i * jointWeightStride), sizeof(glm::vec4));
            }
        }



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

                    // weights
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