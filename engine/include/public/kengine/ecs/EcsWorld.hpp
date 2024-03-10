#pragma once
#include <kengine/ecs/BaseSystem.hpp>

#include <vector>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class EcsWorldConfig {
protected:
    friend class EcsWorld;
    std::unordered_map<std::type_index, void*> services;
    std::vector<std::unique_ptr<BaseSystem>> systems;

public:
    template<typename T>
    EcsWorldConfig& addService(void* service) {
        services[std::type_index(typeid(T))] = service;
        return *this;
    }

    template<typename T>
    EcsWorldConfig& setSystem(void* service) {
        systems.push_back(std::make_unique<T>());
        return *this;
    }
};

class EcsWorld {
private:
    std::unordered_map<std::type_index, void*> services;
    std::vector<std::unique_ptr<BaseSystem>> systems;

public:
    EcsWorld(EcsWorldConfig& wc)
        : services(std::move(wc.services)), systems(std::move(wc.systems)) {
        for (auto& sys : this->systems)
            sys->init();
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