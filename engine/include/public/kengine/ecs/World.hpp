#pragma once
#include <kengine/ecs/BaseSystem.hpp>

#include <vector>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class WorldConfig {
protected:
    friend class World;
    std::unordered_map<std::type_index, void*> services;
    std::unordered_map<std::type_index, std::unique_ptr<BaseSystem>> systems;

public:
    template<typename T>
    WorldConfig& addService(void* service) {
        services[std::type_index(typeid(T))] = service;
        return *this;
    }

    template<typename T, typename... Args>
    WorldConfig& setSystem(Args&&... args) {
        static_assert(std::is_base_of<BaseSystem, T>::value, "T must be derived from BaseSystem");
        systems[std::type_index(typeid(T))] = std::make_unique<T>(std::forward<Args>(args)...);
        return *this;
    }
};

class World {
private:
    std::unordered_map<std::type_index, void*> services;
    std::unordered_map<std::type_index, std::unique_ptr<BaseSystem>> systems;

public:
    World(WorldConfig& wc)
        : services(std::move(wc.services)), systems(std::move(wc.systems)) {
        for (auto& entry : this->systems) {
            auto& sys = entry.second;
            sys->world = this;
            sys->initialize();
        }
    }

    inline static std::unique_ptr<World> create(WorldConfig& wc) {
        return std::make_unique<World>();
    }

    void process(float delta) {
        for (auto& sys : systems)
            sys.second->process(delta);
    }

    template<typename T>
    T* getService() const {
        std::type_index typeIndex = std::type_index(typeid(T));
        auto it = services.find(typeIndex);

        if (it == services.end())
            return nullptr;

        return static_cast<T*>(it->second);
    }

    template<typename T>
    T* getSystem() const {
        std::type_index typeIndex = std::type_index(typeid(T));
        auto it = systems.find(typeIndex);

        if (it == systems.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }
};