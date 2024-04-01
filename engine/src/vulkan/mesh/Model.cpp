#include <kengine/vulkan/mesh/Model.hpp>

MeshGroup::MeshGroup(uint32_t nodeIdx, uint32_t meshCount)
    : nodeIdx(nodeIdx) {
    meshes.reserve(meshCount);
}

void MeshGroup::addMesh(std::unique_ptr<Mesh>&& mesh) {
    meshes.push_back(std::move(mesh));
}

//Model::Model(std::unique_ptr<Mesh>&& mesh) {
//    auto node = std::make_shared<ModelNode>("Main Node");
//    rootNode = node;
//
//    bounds = mesh->getBounds();
//
//    auto modelMesh = std::make_unique<ModelMesh>(std::move(mesh));
//
//    // add to quick access
//   // meshes.push_back(modelMesh.get());
//
//    node->addMesh(std::move(modelMesh));
//}

Model::Model(std::vector<std::shared_ptr<Spatial>>&& nodes,
    std::vector<int16_t>&& parentIndices,
    std::vector<std::unique_ptr<MeshGroup>>&& meshGroups,
    std::vector<uint32_t>&& bones)
    : nodes(std::move(nodes)), parentIndices(std::move(parentIndices)), meshGroups(std::move(meshGroups)), bones(std::move(bones)) {
    rootNode = std::make_shared<ModelNode>("Main Node");
    fillRoot(rootNode, this->nodes);

    // calc bounds
    {
        glm::vec3 min(std::numeric_limits<float>::infinity());
        glm::vec3 max(-std::numeric_limits<float>::infinity());

        glm::vec3 t0{};
        glm::vec3 t1{};
        for (auto& meshGroup : this->meshGroups) {
            for (auto& mesh : meshGroup->getMeshes()) {
                mesh->getBounds().getAabb().getMinMax(t0, t1);
                min = glm::min(min, t0);
                max = glm::max(max, t1);
            }
        }

        bounds = Bounds::fromMinMax(min, max);
        auto i = 0;
    }
}

Model::Model(std::unique_ptr<Mesh>&& mesh) {
    rootNode = std::make_shared<ModelNode>("Main Node");
    nodes.push_back(rootNode);

    bounds = mesh->getBounds();

    auto meshGroup = std::make_unique<MeshGroup>(0, 1);
    meshGroup->addMesh(std::move(mesh));
    meshGroups.push_back(std::move(meshGroup));
}

void Model::fillRoot(std::shared_ptr<Spatial> root, const std::vector<std::shared_ptr<Spatial>> nodes) {
    for (const auto& node : nodes) {
        if (!node->getParent()) {
            root->addChild(node);
            return;
        }

        if (node->hasChildren())
            fillRoot(root, node->getChildren());
    }
}