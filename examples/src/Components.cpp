
#include <kengine/game/components/Components.hpp>
#include <kengine/SceneGraph.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <memory>

std::shared_ptr<Spatial> Component::Spatials::generate(SceneGraph& sceneGraph, Model& model, std::string name) {
    const auto& modelNodes = model.getNodes();
    const auto& parentIndices = model.getParentIndices();
    const auto& meshGroups = model.getMeshGroups();

    std::vector<std::shared_ptr<Spatial>> nodes;
    nodes.reserve(modelNodes.size());

    // copy node details
    for (const auto& n : modelNodes) {
        auto s = sceneGraph.create(n->getName());
        s->setLocalTransform(n->getPosition(), n->getScale(), n->getRotation());
        nodes.push_back(std::move(s));
    }

    auto rootSpatial = sceneGraph.create(name);
    rootSpatialId = rootSpatial->getSceneId();

    // apply parenting
    for (auto i = 0; i < parentIndices.size(); i++) {
        auto pIdx = parentIndices[i];

        if (pIdx == -1) {
            rootSpatial->addChild(nodes[i]);
            continue;
        }

        nodes[static_cast<uint32_t>(pIdx)]->addChild(nodes[i]);
    }

    // insert target nodes for meshes
    meshSpatialsIds.reserve(meshGroups.size());
    for (const auto& mg : meshGroups) {
        auto meshCount = mg->getMeshCount();
        // track each mesh in group to the group's target node
        for (auto i = 0; i < meshCount; i++)
            meshSpatialsIds.push_back(nodes[mg->getNodeIndex()]->getSceneId());
    }

    return rootSpatial;
}
