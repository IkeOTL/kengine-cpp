#pragma once
#include <kengine/Spatial.hpp>
#include <kengine/vulkan/mesh/ModelNode.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <memory>
#include <kengine/Bounds.hpp>

/// <summary>
/// Equivalent to a "mesh" in GLTF2, GLTF2's usage of the term "mesh" doesnt match what you typically see
/// </summary>
class MeshGroup {
private:
    std::vector<std::unique_ptr<Mesh>> meshes;

    MeshGroup(uint32_t meshCount);

public:
    void addMesh(std::unique_ptr<Mesh>&& mesh);
};

class Model {
private:
    std::shared_ptr<Spatial> rootNode = nullptr;
    std::vector<std::shared_ptr<ModelNode>> nodes{};
    std::vector<ModelMesh*> meshes{}; // references the meshes in nodes for faster access
    std::vector<std::shared_ptr<Bone>> bones{};
    Bounds bounds{};

    static void fillRoot(std::shared_ptr<Spatial> root, const std::vector<std::shared_ptr<Spatial>> nodes);
    static void flattenMeshes(const std::vector<std::shared_ptr<Spatial>> nodes, std::vector<ModelMesh*> meshes);

public:
    Model(std::unique_ptr<Mesh>&& mesh);
    Model(std::vector<std::shared_ptr<ModelNode>>&& nodes);

};


