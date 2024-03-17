#pragma once
#include <string>
#include <memory>
#include <vector>

class SceneGraph;
class Model;

namespace Component {


    struct Renderable {
        enum RenderableType {
            DYNAMIC_MODEL,
            //  DYNAMIC_MESH,
            STATIC_MODEL,
            //STATIC_MESH
        };

    public:
        RenderableType type = DYNAMIC_MODEL;
        bool integrateRendering = true;

        void setStatic() {
            type = RenderableType::STATIC_MODEL;
            integrateRendering = false;
        }
    };

    struct Spatial {
    public:
        uint32_t rootSpatialId;
        std::vector<uint32_t> meshSpatialsIds;

        std::shared_ptr<Spatial> generate(SceneGraph sceneGraph, Model model, std::string name);
    private:

    };

}