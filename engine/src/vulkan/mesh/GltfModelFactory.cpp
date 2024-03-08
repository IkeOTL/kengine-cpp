#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include <kengine/vulkan/mesh/GltfModelFactory.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/io/AssetIO.hpp>

thread_local tinygltf::TinyGLTF GltfModelFactory::gltfLoader{};

std::unique_ptr<Model> GltfModelFactory::loadModel(std::string meshKey, int vertexAttributes) {
    std::shared_ptr<Spatial> rootNode = nullptr;

    tinygltf::Model model;
    std::string err, warn;

    auto assetData = assetIo.loadBuffer(meshKey);

    auto ret = gltfLoader.LoadBinaryFromMemory(&model, &err, &warn, assetData->data(), assetData->length());

    // find all joint indices
    std::unordered_set<uint32_t> jointNodeIndices;
    for (const auto& skin : model.skins)
        for (const auto jointIndex : skin.joints)
            jointNodeIndices.insert(jointIndex);

    std::vector<std::shared_ptr<Spatial>> nodes;
    nodes.reserve(model.nodes.size());
    for (size_t i = 0; i < model.nodes.size(); i++)
    {
        if (jointNodeIndices.find(i) != jointNodeIndices.end()) {
            // make bone spatial
            continue;
        }

        // make normal spatial
    }

    // meshes that we should actually load
    std::unordered_set<int> meshGroupIndices{};

    // load node heirarchy
    for (size_t i = 0; i < model.scenes[model.defaultScene].nodes.size(); i++)
        processNode(model, model.scenes[model.defaultScene].nodes[i], meshGroupIndices);

    // load meshes
    std::unordered_map<int, std::unique_ptr<MeshGroup>> meshGroups{};
    for (auto& meshGroupIdx : meshGroupIndices)
        loadMeshGroup(model, meshGroupIdx, meshGroups, vertexAttributes);

    return std::unique_ptr<Model>();
}

void GltfModelFactory::processNode(const tinygltf::Model& model, int nodeIndex, std::unordered_set<int>& meshGroupIndices) const {
    const tinygltf::Node& node = model.nodes[nodeIndex];

    // if a node we touch has a mesh group we mark it for loading
    if (node.mesh != -1)
        meshGroupIndices.insert(node.mesh);

    for (int childIndex : node.children)
        processNode(model, childIndex, meshGroupIndices);
}

void GltfModelFactory::loadMeshGroup(const tinygltf::Model& model, int meshGroupIdx, std::unordered_map<int, std::unique_ptr<MeshGroup>>& meshGroups, int vertexAttributes) const {
    auto& meshGroupData = model.meshes[meshGroupIdx];

    auto meshCount = meshGroupData.primitives.size();
    auto& meshGroup = meshGroups[meshGroupIdx] = std::make_unique<MeshGroup>(meshCount);
    for (auto i = 0; i < meshCount; i++) {
        if ((vertexAttributes & VertexAttribute::TEX_COORDS) == 0) {
            MeshBuilder<Vertex> mb(vertexAttributes);
            loadMesh<Vertex>(model, mb, meshGroupData.primitives[i], *meshGroup);
            continue;
        }

        if (vertexAttributes & VertexAttribute::SKELETON) {
            MeshBuilder<RiggedTexturedVertex> mb(vertexAttributes);
            loadMesh(model, mb, meshGroupData.primitives[i], *meshGroup);
            continue;
        }

        if (vertexAttributes & VertexAttribute::NORMAL) {
            MeshBuilder<TexturedVertex> mb(vertexAttributes);
            loadMesh<TexturedVertex>(model, mb, meshGroupData.primitives[i], *meshGroup);
            continue;
        }

        MeshBuilder<SimpleTexturedVertex> mb(vertexAttributes);
        loadMesh<SimpleTexturedVertex>(model, mb, meshGroupData.primitives[i], *meshGroup);
    }
}

