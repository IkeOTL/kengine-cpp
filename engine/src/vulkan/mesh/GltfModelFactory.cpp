#include <kengine/vulkan/mesh/GltfModelFactory.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <kengine/io/AssetIO.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ke {
    thread_local tinygltf::TinyGLTF GltfModelFactory::gltfLoader{};

    std::unique_ptr<Model> GltfModelFactory::loadModel(const ModelConfig& config) {
        tinygltf::Model model;
        std::string err, warn;

        auto assetData = assetIo.load(config.getModelKey());

        auto ret = gltfLoader.LoadBinaryFromMemory(&model, &err, &warn, assetData->data(), assetData->length());

        std::vector<std::shared_ptr<Spatial>> spatialNodes;
        std::vector<std::shared_ptr<Bone>> spatialBones;
        std::vector<uint32_t> bonesNodeIndices;
        std::vector<int16_t> parentIndices;

        // load nodes and bones
        {
            std::vector<glm::mat4> inverseBindMatrices;
            if ((config.getAttributes() & VertexAttribute::SKELETON) && model.skins[0].inverseBindMatrices > -1) {
                const auto& accessor = model.accessors[model.skins[0].inverseBindMatrices];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const unsigned char* inverseBindMatricesData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

                auto stride = bufferView.byteStride > 0 ? bufferView.byteStride : sizeof(glm::mat4);

                inverseBindMatrices.resize(accessor.count);
                for (size_t i = 0; i < accessor.count; ++i)
                    memcpy(&inverseBindMatrices[i], inverseBindMatricesData + (i * stride), sizeof(glm::mat4));
            }

            // create node entries
            spatialNodes.reserve(model.nodes.size());
            parentIndices.resize(model.nodes.size(), -1);

            std::unordered_map<int, int> boneMap;

            if (config.getAttributes() & VertexAttribute::SKELETON) {
                // find all joint indices for first skin
                auto& joints = model.skins[0].joints;
                spatialBones.reserve(joints.size());

                bonesNodeIndices.resize(joints.size());
                for (auto i = 0; i < joints.size(); i++) {
                    bonesNodeIndices[i] = joints[i];
                    boneMap[joints[i]] = i;
                }
            }

            // load all nodes to keep it simple
            for (size_t i = 0; i < model.nodes.size(); i++) {
                auto& node = model.nodes[i];

                // normal node, aka not a joint
                auto boneIdx = boneMap.find(i);
                if (boneIdx == boneMap.end()) {
                    auto spatial = std::make_shared<Spatial>("node: (" + std::to_string(i) + ") " + node.name);

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

                auto boneSpatial = std::make_shared<Bone>(boneIdx->second, model.nodes[i].name);
                //make sure right bind matrix is going to right bone!
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
        std::unordered_map<int, std::unique_ptr<MeshGroup>> meshGroups{};

        // load node heirarchy, and apply parenting
        for (size_t i = 0; i < model.scenes[model.defaultScene].nodes.size(); i++)
            processNode(model, model.scenes[model.defaultScene].nodes[i], meshGroupIndices, meshGroups, spatialNodes, parentIndices);

        // load meshes
        for (auto meshGroupIdx : meshGroupIndices)
            loadMeshGroup(model, meshGroupIdx, meshGroups, config.getAttributes());

        std::vector<std::unique_ptr<MeshGroup>> listMeshGroups;
        listMeshGroups.reserve(meshGroups.size());
        for (auto& mge : meshGroups)
            listMeshGroups.push_back(std::move(mge.second));

        return std::make_unique<Model>(std::move(spatialNodes), std::move(parentIndices), std::move(listMeshGroups), std::move(bonesNodeIndices));
    }

    void GltfModelFactory::processNode(const tinygltf::Model& model, int nodeIndex, std::unordered_set<int>& meshGroupIndices,
        std::unordered_map<int, std::unique_ptr<MeshGroup>>& meshGroups, std::vector<std::shared_ptr<Spatial>>& spatialNodes, std::vector<int16_t>& parentIndices) const {
        const auto& node = model.nodes[nodeIndex];
        auto& parentSpatial = spatialNodes[nodeIndex];

        // if a node we touch has a mesh group we mark it for loading
        if (node.mesh != -1) {
            meshGroupIndices.insert(node.mesh);
            auto& meshGroupData = model.meshes[node.mesh];

            // create mesh group entry
            auto meshCount = meshGroupData.primitives.size();
            auto& meshGroup = meshGroups[node.mesh] = std::make_unique<MeshGroup>(nodeIndex, meshCount);
        }

        for (auto childIndex : node.children) {
            parentIndices[childIndex] = nodeIndex;
            parentSpatial->addChild(spatialNodes[childIndex]);
            processNode(model, childIndex, meshGroupIndices, meshGroups, spatialNodes, parentIndices);
        }
    }

    void GltfModelFactory::loadMeshGroup(const tinygltf::Model& model, int meshGroupIdx, std::unordered_map<int, std::unique_ptr<MeshGroup>>& meshGroups, int vertexAttributes) const {
        auto& meshGroupData = model.meshes[meshGroupIdx];

        auto meshCount = meshGroupData.primitives.size();
        auto& meshGroup = meshGroups[meshGroupIdx];
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
} // namespace ke

