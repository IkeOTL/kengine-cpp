#pragma once
#include <kengine/Spatial.hpp>
#include <kengine/vulkan/mesh/ModelNode.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <memory>
#include <kengine/Bounds.hpp>

class Model {
private:
    std::shared_ptr<Spatial> rootNode = nullptr;
    std::vector<std::shared_ptr<ModelNode>> nodes{};
    std::vector<ModelMesh*> meshes{}; // references the meshes in nodes for faster access
    std::vector<std::shared_ptr<Bone>> bones{};
    Bounds bounds{};

public:
    Model(std::unique_ptr<Mesh>&& mesh);
    Model(std::vector<std::shared_ptr<ModelNode>>&& nodes);
};
