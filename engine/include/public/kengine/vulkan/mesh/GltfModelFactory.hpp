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

    template<typename T>
    const T* getAttrBuffer(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string attributeName, uint32_t& count) const {
        auto it = primitive.attributes.find(attributeName);
        if (it == primitive.attributes.end())
            throw new std::runtime_error("Vertex attribute not found.");

        auto accessorIndex = it->second;
        const auto& accessor = model.accessors[accessorIndex];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];
        const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;

        // is accessor.count obj count or byte count?
        count = accessor.count;
        return reinterpret_cast<const T*>(&buffer.data[byteOffset]);
    }

    template <typename V>
    void loadMesh(const tinygltf::Model& model, MeshBuilder<V>& mb, const tinygltf::Primitive& primitive, MeshGroup& meshGroup) const {
        // load indices
        {
            auto indicesIdx = primitive.indices;

            if (indicesIdx == -1)
                throw new std::runtime_error("Currently only support indexed meshes.");

            const auto& accessor = model.accessors[indicesIdx];

            if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                throw new std::runtime_error("Component type not handled.");

            // resize mesh builder
            {
                const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                mb.resize(accessor.count, posAccessor.count);
            }

            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];

            // Accessor could define a byte offset within the bufferView
            const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
            const auto indices = reinterpret_cast<const unsigned short*>(&(buffer.data[byteOffset]));

            // assume builder has had resize() executed
            memcpy(mb.getIndices().data(), indices, sizeof(uint16_t) * accessor.count);
        }

        // load vertices
        {
            auto vertexAttributes = mb.getVertexAttributes();
            if (vertexAttributes & VertexAttribute::POSITION) {
                uint32_t count;
                const auto attr = getAttrBuffer<glm::vec3>(model, primitive, "POSITION", count);
                for (int32_t i = 0; i < count; i++)
                {

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