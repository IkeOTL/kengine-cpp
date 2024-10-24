#pragma once
#include <kengine/Spatial.hpp>
#include <kengine/vulkan/mesh/ModelNode.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <kengine/Bounds.hpp>

#include <memory>

namespace ke {
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

        uint32_t getNodeIndex() const {
            return nodeIdx;
        }

        const Mesh& getMesh(uint32_t i) const {
            return *meshes[i];
        }

        uint32_t getMeshCount() const {
            return meshes.size();
        }

        const std::vector<std::unique_ptr<Mesh>>& getMeshes() const {
            return meshes;
        }
    };

    class Model {
    private:
        std::shared_ptr<Spatial> rootNode = nullptr;
        std::vector<std::shared_ptr<Spatial>> nodes;
        std::vector<int16_t> parentIndices;
        std::vector<std::unique_ptr<MeshGroup>> meshGroups;
        std::vector<uint32_t> bones;
        Bounds bounds{};

        static void fillRoot(std::shared_ptr<Spatial> root, const std::vector<std::shared_ptr<Spatial>> nodes);

    public:
        Model(std::unique_ptr<Mesh>&& mesh);

        Model(std::vector<std::shared_ptr<Spatial>>&& nodes,
            std::vector<int16_t>&& parentIndices,
            std::vector<std::unique_ptr<MeshGroup>>&& meshGroups,
            std::vector<uint32_t>&& bones
        );

        const std::vector<std::shared_ptr<Spatial>>& getNodes() const {
            return nodes;
        }

        const std::vector<uint32_t>& getBoneIndices() const {
            return bones;
        }

        const std::vector<int16_t>& getParentIndices() const {
            return parentIndices;
        }

        const std::vector<std::unique_ptr<MeshGroup>>& getMeshGroups() const {
            return meshGroups;
        }

        const Bounds& getBounds() const {
            return bounds;
        }

        std::shared_ptr<Skeleton> acquireSkeleton(std::string name);
    };
} // namespace ke