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
    std::unordered_map<int, std::unique_ptr<MeshGroup>>&& meshGroups,
    std::vector<uint32_t>&& bones)
    : nodes(std::move(nodes)), meshGroups(std::move(meshGroups)), bones(std::move(bones)) {
    rootNode = std::make_shared<ModelNode>("Main Node");
    fillRoot(rootNode, this->nodes);

    // calc bounds
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