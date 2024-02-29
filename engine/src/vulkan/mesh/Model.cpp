#include <kengine/vulkan/mesh/Model.hpp>

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

}
