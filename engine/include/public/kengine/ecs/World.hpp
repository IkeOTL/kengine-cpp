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
    std::vector<std::unique_ptr<BaseSystem>> systems;

public:
    template<typename T>
    WorldConfig& addService(void* service) {
        services[std::type_index(typeid(T))] = service;
        return *this;
    }

    template<typename T>
    WorldConfig& setSystem() {
        static_assert(std::is_base_of<BaseSystem, T>::value, "T must be derived from BaseSystem");
        systems.push_back(std::make_unique<T>());
        return *this;
    }
};

class World {
private:
    std::unordered_map<std::type_index, void*> services;
    std::vector<std::unique_ptr<BaseSystem>> systems;

public:
    World(WorldConfig& wc)
        : services(std::move(wc.services)), systems(std::move(wc.systems)) {
        for (auto& sys : this->systems) {
            sys->world = this;
            sys->init();
        }
    }

    template<typename T>
    T* getService() const {
        std::type_index typeIndex = std::type_index(typeid(T));
        auto it = services.find(typeIndex);

        if (it == services.end())
            throw std::runtime_error("Service not found.");

        return static_cast<T*>(it->second);
    }
};