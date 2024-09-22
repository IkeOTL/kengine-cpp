
#include "components/Components.hpp"

#include <kengine/SceneGraph.hpp>
#include <kengine/vulkan/mesh/Model.hpp>
#include <memory>

std::shared_ptr<Spatial> Component::Spatials::generate(SceneGraph& sceneGraph, Model& model, std::string name,
    Renderable::RenderableType renderType) {
    const auto& modelNodes = model.getNodes();
    const auto& parentIndices = model.getParentIndices();
    const auto& meshGroups = model.getMeshGroups();
    const auto& boneIndices = model.getBoneIndices();

    spatialsIds.reserve(modelNodes.size());

    std::vector<std::shared_ptr<Spatial>> nodes;
    nodes.reserve(modelNodes.size());

    std::unordered_map<uint32_t, uint32_t> boneMap;
    for (int i = 0; i < boneIndices.size(); i++)
        boneMap[boneIndices[i]] = i;

    // copy node details
    for (int i = 0; i < modelNodes.size(); i++) {
        const auto& n = modelNodes[i];

        // normal spatial
        auto it = boneMap.find(i);
        if (it == boneMap.end()) {
            auto s = sceneGraph.create(n->getName());
            s->setLocalTransform(n->getPosition(), n->getScale(), n->getRotation());

            // save id in component since we'll need it for various operations (like skeleton bone indices)
            spatialsIds.push_back(s->getSceneId());

            nodes.push_back(std::move(s));
            continue;
        }

        // bone spatial
        {
            auto ogBone = std::static_pointer_cast<Bone>(n);
            auto s = std::make_shared<Bone>(ogBone->getBoneId(), n->getName());

            s->setLocalTransform(n->getPosition(), n->getScale(), n->getRotation());
            s->setInverseBindWorldMatrix(ogBone->getInverseBindWorldMatrix());
            s->saveBindPose();

            sceneGraph.add(s);

            // save id in component since we'll need it for various operations (like skeleton bone indices)
            spatialsIds.push_back(s->getSceneId());

            nodes.push_back(std::move(s));
        }
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

    if (renderType == Renderable::RenderableType::DYNAMIC_MODEL)
        previousTransforms.resize(meshSpatialsIds.size());

    return rootSpatial;
}


/// <summary>
/// create a skeleton based on spatials from the generated model spatial hierarchy
/// </summary>
std::shared_ptr<Skeleton> Component::SkeletonComp::generate(SceneGraph& sceneGraph, Model& model, Component::Spatials& spatials, std::string name) {
    auto& boneIndices = model.getBoneIndices();

    std::vector<std::shared_ptr<Bone>> bones;
    bones.reserve(boneIndices.size());

    for (auto i : boneIndices)
        bones.push_back(std::static_pointer_cast<Bone>(sceneGraph.get(spatials.spatialsIds[i])));

    return std::make_shared<Skeleton>(name, std::move(bones));
}