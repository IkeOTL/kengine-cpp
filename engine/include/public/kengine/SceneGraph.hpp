#pragma once
#include <kengine/Spatial.hpp>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

class SceneGraph {
private:
    std::unordered_map<int, std::shared_ptr<Spatial>> spatialCache;
    uint32_t runningId = 0;
    std::shared_mutex mtx;

public:
    inline static std::unique_ptr<SceneGraph> create() {
        return std::make_unique<SceneGraph>();
    }

    std::shared_ptr<Spatial> create(std::string name);
    uint32_t add(std::shared_ptr<Spatial> spatial);
    std::shared_ptr<Spatial> get(uint32_t id);
    std::shared_ptr<Spatial> remove(Spatial& s);
};