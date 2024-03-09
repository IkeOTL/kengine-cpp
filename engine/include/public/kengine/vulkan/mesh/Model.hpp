#pragma once
#include <kengine/Spatial.hpp>
#include <kengine/vulkan/mesh/ModelNode.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <kengine/Bounds.hpp>

#include <memory>

/// <summary>
/// Equivalent to a "mesh" in GLTF2, GLTF2's usage of the term "mesh" doesnt match what you typically see
/// </summary>
class MeshGroup {
private:
    const uint32_t nodeIdx;
    std::vector<std::unique_ptr<Mesh>> meshes;

public:
    MeshGroup(uint32_t nodeIdx, uint32_t meshCount);

    void addMesh(std::unique_ptr<Mesh>&& mesh);

    uint32_t getNodeIndex() {
        return nodeIdx;
    }
};

class Model {
private:
    std::shared_ptr<Spatial> rootNode = nullptr;
    std::vector<std::shared_ptr<Spatial>> nodes;
    std::unordered_map<int, std::unique_ptr<MeshGroup>> meshGroups;
    std::vector<uint32_t> bones;
    Bounds bounds{};

    static void fillRoot(std::shared_ptr<Spatial> root, const std::vector<std::shared_ptr<Spatial>> nodes);

public:
    //Model(std::unique_ptr<Mesh>&& mesh);
    //Model(std::vector<std::shared_ptr<ModelNode>>&& nodes);
    Model(std::vector<std::shared_ptr<Spatial>>&& nodes,
        std::unordered_map<int, std::unique_ptr<MeshGroup>>&& meshGroups,
        std::vector<uint32_t>&& bones
    );
};


