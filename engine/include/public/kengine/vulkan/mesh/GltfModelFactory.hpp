#pragma once
#include <kengine/vulkan/mesh/ModelFactory.hpp>
#include <kengine/io/AssetIO.hpp>
#include <thirdparty/tiny_gltf.h>
#include <unordered_set>
#include <thread>

class VulkanContext;
class assetIo;
class MeshBuilder;

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
    void loadMesh(const tinygltf::Model& model, const tinygltf::Primitive& meshPrimitive, MeshGroup& meshGroup, int vertexAttributes) const;
    void readIndices(MeshBuilder& mb, const tinygltf::Primitive& meshPrimitive);
};