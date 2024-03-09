#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include <kengine/vulkan/mesh/GltfModelFactory.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/io/AssetIO.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

thread_local tinygltf::TinyGLTF GltfModelFactory::gltfLoader{};

std::unique_ptr<Model> GltfModelFactory::loadModel(std::string meshKey, int vertexAttributes) {
    std::shared_ptr<Spatial> rootNode = nullptr;

    tinygltf::Model model;
    std::string err, warn;

    auto assetData = assetIo.loadBuffer(meshKey);

    auto ret = gltfLoader.LoadBinaryFromMemory(&model, &err, &warn, assetData->data(), assetData->length());

    std::vector<std::shared_ptr<Spatial>> spatialNodes;
    std::vector<std::shared_ptr<Bone>> spatialBones;

    // load nodes and bones
    {
        std::vector<glm::mat4> inverseBindMatrices;
        if (model.skins[0].inverseBindMatrices > -1) {
            const auto& accessor = model.accessors[model.skins[0].inverseBindMatrices];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            const unsigned char* inverseBindMatricesData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

            inverseBindMatrices.resize(accessor.count);
            for (size_t i = 0; i < accessor.count; ++i)
                memcpy(&(inverseBindMatrices[i]), inverseBindMatricesData + (i * bufferView.byteStride), sizeof(glm::mat4));
        }

        // find all joint indices for first skin
        std::unordered_set<uint32_t> jointNodeIndices;
        if (!model.skins.empty())
            for (const auto jointIndex : model.skins[0].joints)
                jointNodeIndices.insert(jointIndex);

        // create node entries
        spatialNodes.reserve(model.nodes.size());

        spatialBones.reserve(jointNodeIndices.size());

        // load all nodes to keep it simple
        for (size_t i = 0; i < model.nodes.size(); i++) {
            auto& node = model.nodes[i];

            // normal node, aka not a joint
            if (jointNodeIndices.find(i) == jointNodeIndices.end()) {
                auto spatial = std::make_shared<Spatial>("node: " + i);

                if (!node.translation.empty()) {
                    glm::dvec3 pos;
                    memcpy(&pos, node.translation.data(), sizeof(glm::dvec3));
                    spatial->setLocalPosition(glm::vec3(pos));
                }

                if (!node.rotation.empty()) {
                    glm::dquat rot;
                    memcpy(&rot, node.rotation.data(), sizeof(glm::dquat));
                    spatial->setLocalRotation(glm::quat(rot));
                }

                if (!node.scale.empty()) {
                    glm::dvec3 scl;
                    memcpy(&scl, node.scale.data(), sizeof(glm::dvec3));
                    spatial->setLocalScale(glm::vec3(scl));
                }

                spatialNodes.push_back(spatial);

                continue;
            }

            auto boneSpatial = std::make_shared<Bone>(spatialBones.size(), model.nodes[i].name);
            boneSpatial->setInverseBindWorldMatrix(inverseBindMatrices[boneSpatial->getBoneId()]);


            if (!node.translation.empty()) {
                glm::dvec3 pos;
                memcpy(&pos, node.translation.data(), sizeof(glm::dvec3));
                boneSpatial->setLocalPosition(glm::vec3(pos));
            }

            if (!node.rotation.empty()) {
                glm::dquat rot;
                memcpy(&rot, node.rotation.data(), sizeof(glm::dquat));
                boneSpatial->setLocalRotation(glm::quat(rot));
            }

            if (!node.scale.empty()) {
                glm::dvec3 scl;
                memcpy(&scl, node.scale.data(), sizeof(glm::dvec3));
                boneSpatial->setLocalScale(glm::vec3(scl));
            }

            boneSpatial->saveBindPose();

            spatialNodes.push_back(boneSpatial);
            spatialBones.push_back(boneSpatial);
        }
    }

    // meshes that we should actually load
    std::unordered_set<int> meshGroupIndices{};

    // load node heirarchy
    for (size_t i = 0; i < model.scenes[model.defaultScene].nodes.size(); i++)
        processNode(model, model.scenes[model.defaultScene].nodes[i], meshGroupIndices, spatialNodes);

    // load meshes
    std::unordered_map<int, std::unique_ptr<MeshGroup>> meshGroups{};
    for (auto& meshGroupIdx : meshGroupIndices)
        loadMeshGroup(model, meshGroupIdx, meshGroups, vertexAttributes);

    return std::unique_ptr<Model>();
}

void GltfModelFactory::processNode(const tinygltf::Model& model, int nodeIndex, std::unordered_set<int>& meshGroupIndices, std::vector<std::shared_ptr<Spatial>>& spatialNodes) const {
    const auto& node = model.nodes[nodeIndex];
    auto& parentSpatial = spatialNodes[nodeIndex];

    // if a node we touch has a mesh group we mark it for loading
    if (node.mesh != -1)
        meshGroupIndices.insert(node.mesh);

    for (auto childIndex : node.children) {
        parentSpatial->addChild(spatialNodes[childIndex]);
        processNode(model, childIndex, meshGroupIndices, spatialNodes);
    }
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

