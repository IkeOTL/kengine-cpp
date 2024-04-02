#pragma once
#include <kengine/FrustumIntersection.hpp>
#include <thirdparty/entt.hpp>
#include <glm/glm.hpp>
#include <shared_mutex>
#include <kengine/Bounds.hpp>
#include <kengine/Transform.hpp>

class SpatialGrid {
public:
    static const int MAX_CELLS_PER_ENTITY = 4;

    struct SpatialGridUpdate {
        entt::entity entity;
        const glm::mat4& transform;
        const Aabb& bounds;
    };

private:
    const uint32_t worldWidth, worldLength, cellSize;
    const uint32_t cellCountX, cellCountZ;
    const int32_t worldOffsetX, worldOffsetZ;

    std::unordered_map<entt::entity, std::array<uint32_t, MAX_CELLS_PER_ENTITY>> entityIndex;
    std::vector<std::vector<entt::entity>> cells;
    std::unordered_set<entt::entity> dirtySet;

    std::shared_mutex lock{};

    glm::vec3 intersectPoint(const glm::vec3& start, const glm::vec3& end);

public:
    SpatialGrid(uint32_t worldWidth, uint32_t worldLength, uint32_t cellSize);

    void setDirty(const entt::entity entity);

    void getVisible(const glm::vec3& camPos, const std::array<glm::vec3, 8>& frustomPoints,
        const FrustumIntersection& frustumTester, std::vector<entt::entity>& dest);

    void updateEntity(entt::entity entityId, const glm::mat4& xform, const Aabb& aabb);
    void addEntity(entt::entity entityId, const glm::mat4& xform, const Aabb& aabb);
    void removeEntity(entt::entity entityId);
    void processDirtyEntities(std::function<SpatialGridUpdate(entt::entity)> func);
};