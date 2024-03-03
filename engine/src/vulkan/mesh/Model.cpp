#include <kengine/vulkan/mesh/Model.hpp>

MeshGroup::MeshGroup(uint32_t meshCount) {
    meshes.reserve(meshCount);
}

void MeshGroup::addMesh(std::unique_ptr<Mesh>&& mesh) {
    meshes.push_back(std::move(mesh));
}

Model::Model(std::unique_ptr<Mesh>&& mesh) {
    auto node = std::make_shared<ModelNode>("Main Node");
    rootNode = node;

    bounds = mesh->getMeshData().getBounds();

    auto modelMesh = std::make_unique<ModelMesh>(std::move(mesh));

    // add to quick access
    meshes.push_back(modelMesh.get());

    node->addMesh(std::move(modelMesh));
}

Model::Model(std::vector<std::shared_ptr<ModelNode>>&& nodes)
    : nodes(std::move(nodes)) {
    rootNode = std::make_shared<ModelNode>("Main Node");

    std::vector<std::shared_ptr<Spatial>> spatials;
    spatials.reserve(this->nodes.size());

    for (const auto& modelNode : this->nodes)
        spatials.push_back(std::static_pointer_cast<Spatial>(modelNode));

    fillRoot(rootNode, spatials);
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

void Model::flattenMeshes(const std::vector<std::shared_ptr<Spatial>> nodes, std::vector<ModelMesh*> meshes) {
    for (const auto& sNode : nodes) {
        auto node = std::static_pointer_cast<ModelNode>(sNode);

        auto& nodeMeshes = node->getMeshes();
        for (const auto& mesh : nodeMeshes)
            meshes.push_back(mesh.get());

        if (sNode->hasChildren())
            flattenMeshes(sNode->getChildren(), meshes);
    }
}
