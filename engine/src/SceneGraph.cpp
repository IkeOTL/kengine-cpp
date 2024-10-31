#include <kengine/SceneGraph.hpp>
#include <tracy/Tracy.hpp>

namespace ke {
    std::shared_ptr<Spatial> SceneGraph::create(std::string name) {
        std::lock_guard<std::shared_mutex> lock(mtx);
        auto s = std::make_shared<Spatial>(name);
        s->setSceneId(runningId++);
        spatialCache[s->getSceneId()] = s;
        return s;
    }

    uint32_t SceneGraph::add(std::shared_ptr<Spatial> s) {
        std::lock_guard<std::shared_mutex> lock(mtx);
        auto id = runningId++;
        s->setSceneId(id);
        spatialCache[id] = s;

        auto children = s->getChildren();
        if (!children.empty())
            for (auto& child : children)
                add(child);

        return id;
    }

    std::shared_ptr<Spatial> SceneGraph::get(uint32_t id) {
        ZoneScoped;

        std::shared_lock<std::shared_mutex> lock(mtx); // allow multiple thread reads

        auto it = spatialCache.find(id);
        if (it == spatialCache.end())
            return nullptr;

        return it->second;
    }

    std::shared_ptr<Spatial> SceneGraph::remove(Spatial& s) {
        std::lock_guard<std::shared_mutex> lock(mtx);
        if (s.getSceneId() == -1)
            throw std::runtime_error("This is not a scene graph spatial.");

        auto it = spatialCache.find(s.getSceneId());
        if (it == spatialCache.end())
            return nullptr;

        auto removed = std::move(it->second);
        spatialCache.erase(it);

        removed->setSceneId(-1);

        auto parent = removed->getParent();
        if (parent)
            parent->removeChild(removed->getName());

        auto children = removed->getChildren();
        if (children.empty())
            return removed;

        for (auto c : children)
            remove(*c);

        return removed;
    }
} // namespace ke