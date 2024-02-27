#include <kengine/vulkan/mesh/Model.hpp>

Model::Model(std::unique_ptr<Mesh>&& mesh) {

}

Model::Model(std::vector<std::shared_ptr<ModelNode>>&& nodes)
    : nodes(std::move(nodes)) {

}
