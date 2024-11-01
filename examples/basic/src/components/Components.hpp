#pragma once
#include <kengine/SceneGraph.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <string>
#include <memory>
#include <vector>

namespace ke {
    class Model;
}

namespace Component {
    struct Renderable {
        enum RenderableType {
            DYNAMIC_MODEL,
            //  DYNAMIC_MESH,
            STATIC_MODEL,
            // STATIC_MESH
        };

        RenderableType type = DYNAMIC_MODEL;
        bool integrateRendering = true;

        void setStatic() {
            type = RenderableType::STATIC_MODEL;
            integrateRendering = false;
        }
    };

    struct Spatials {
        uint32_t rootSpatialId;
        std::vector<uint32_t> spatialsIds;
        std::vector<uint32_t> meshSpatialsIds;
        std::vector<ke::Transform> previousTransforms;

        std::shared_ptr<ke::Spatial> generate(ke::SceneGraph& sceneGraph, ke::Model& model, std::string name, Renderable::RenderableType renderType);
    };

    struct SkeletonComp {
        uint32_t skeletonId;
        uint32_t bufId;

        std::shared_ptr<ke::Skeleton> generate(ke::SceneGraph& sceneGraph, ke::Model& model, Component::Spatials& spatials, std::string name);
    };
} // namespace Component