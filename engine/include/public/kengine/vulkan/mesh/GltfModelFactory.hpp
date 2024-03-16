#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/mesh/ModelFactory.hpp>
#include <kengine/vulkan/mesh/MeshBuilder.hpp>
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

    std::unique_ptr<Model> loadModel(const ModelConfig& config) override;

private:
    void processNode(const tinygltf::Model& model, int nodeIndex,
        std::unordered_set<int>& meshIndices, std::unordered_map<int, std::unique_ptr<MeshGroup>>& meshGroups,
        std::vector<std::shared_ptr<Spatial>>& spatialNodes) const;

    void loadMeshGroup(const tinygltf::Model& model, int meshGroupIdx, std::unordered_map<int,
        std::unique_ptr<MeshGroup>>&mesheGroups, int vertexAttributes) const;

    const tinygltf::Accessor& getAccessor(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string attributeName) const {
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
        return getAttrBuffer<T>(model, accessor, count, stride);
    }

    template <typename V>
    void loadMesh(const tinygltf::Model& model, MeshBuilder<V>& mb, const tinygltf::Primitive& primitive, MeshGroup& meshGroup) const {
        // prepare indices
        auto indicesIdx = primitive.indices;

        if (indicesIdx == -1)
            throw std::runtime_error("Currently only support indexed meshes.");

        const auto& idxAccessor = model.accessors[indicesIdx];

        // prepare verts
        const unsigned char* positionAttr = nullptr;
        const unsigned char* normalAttr = nullptr;
        const unsigned char* texCoordAttr = nullptr;
        const unsigned char* tangentAttr = nullptr;
        const unsigned char* jointIndexAttr = nullptr;
        const unsigned char* jointWeightAttr = nullptr;

        size_t vertCount = 0;
        size_t positionStride = 0, normalStride = 0, texCoordStride = 0,
            tangentStride = 0, jointIndexStride = 0, jointWeightStride = 0;
        int jointComponentType;

        auto vertexAttributes = mb.getVertexAttributes();
        if (vertexAttributes & VertexAttribute::POSITION)
            positionAttr = getAttrBuffer<glm::vec3>(model, primitive, "POSITION", vertCount, positionStride);

        if (vertexAttributes & VertexAttribute::NORMAL)
            normalAttr = getAttrBuffer<glm::vec3>(model, primitive, "NORMAL", vertCount, normalStride);

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

        // resize mesh builder            
        mb.resize(idxAccessor.count, vertCount);

        // load indices
        {
            if (idxAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                && idxAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                throw std::runtime_error("Component type not handled.");

            const auto& bufferView = model.bufferViews[idxAccessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];

            // Accessor could define a byte offset within the bufferView
            const auto byteOffset = idxAccessor.byteOffset + bufferView.byteOffset;
            const auto* meshIndices = &(buffer.data[byteOffset]);

            auto& targetIndices = mb.getIndices();
            // assume builder has had resize() executed
            if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const auto meshIndices = reinterpret_cast<const uint16_t*>(&(buffer.data[byteOffset]));
                for (auto i = 0; i < idxAccessor.count; i++)
                    targetIndices[i] = meshIndices[i];
            }
            else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                memcpy(targetIndices.data(), &(buffer.data[byteOffset]), idxAccessor.count * sizeof(uint32_t));
        }

        // load vertices
        {
            auto& verts = mb.getVertices();
            for (size_t i = 0; i < vertCount; i++) {
                if (positionAttr)
                    memcpy(&(verts[i].position), positionAttr + (i * positionStride), sizeof(glm::vec3));

                if constexpr (std::is_base_of_v<TexturedVertex, V>)
                    if (normalAttr)
                        memcpy(&(verts[i].normal), normalAttr + (i * normalStride), sizeof(glm::vec3));

                if constexpr (std::is_base_of_v<TexturedVertex, V>)
                    if (texCoordAttr)
                        memcpy(&(verts[i].texCoords), texCoordAttr + (i * texCoordStride), sizeof(glm::vec2));

                if constexpr (std::is_base_of_v<TexturedVertex, V>)
                    if (tangentAttr)
                        memcpy(&(verts[i].tangent), tangentAttr + (i * tangentStride), sizeof(glm::vec4));

                // todo: improve strategy to load effeciently
                if constexpr (std::is_base_of_v<RiggedTexturedVertex, V>) {
                    if (jointIndexAttr && jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        glm::u8vec4 tmp;
                        memcpy(&tmp, jointIndexAttr + (i * jointIndexStride), sizeof(glm::u8vec4));
                        verts[i].blendIndex = glm::uvec4(tmp);
                    }
                    else if (jointIndexAttr && jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        glm::u16vec4 tmp;
                        memcpy(&tmp, jointIndexAttr + (i * jointIndexStride), sizeof(glm::u16vec4));
                        verts[i].blendIndex = glm::uvec4(tmp);
                    }

                    if (jointWeightAttr)
                        memcpy(&(verts[i].blendWeight), jointWeightAttr + (i * jointWeightStride), sizeof(glm::vec4));
                }
            }
        }

        meshGroup.addMesh(
            mb.build(&vkContext)
        );
    }

};