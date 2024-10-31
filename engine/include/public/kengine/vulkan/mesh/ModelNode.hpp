#pragma once
#include <kengine/vulkan/mesh/ModelNode.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/Spatial.hpp>
#include <memory>

namespace ke {
    class ModelNode;

    class ModelMesh {
    private:
        ModelNode* parent = nullptr;
        const std::unique_ptr<Mesh> mesh;

    public:
        ModelMesh(std::unique_ptr<Mesh>&& mesh)
            : mesh(std::move(mesh)) {};

        const Mesh& getMesh() const {
            if (!mesh)
                throw std::runtime_error("Mesh was never set.");

            return *mesh;
        }

        const ModelNode* getParent() const {
            return parent;
        }

        void setParent(ModelNode* parent) {
            this->parent = parent;
        }
    };

    class ModelNode : public Spatial {
    private:
        std::vector<std::unique_ptr<ModelMesh>> meshes;

    public:
        ModelNode(std::string name)
            : Spatial(name) {}

        void addMesh(std::unique_ptr<ModelMesh>&& m) {
            m->setParent(this);
            meshes.push_back(std::move(m));
        }

        const std::vector<std::unique_ptr<ModelMesh>>& getMeshes() const {
            return meshes;
        }
    };
} // namespace ke