#pragma once
#include <thirdparty/entt.hpp>
#include <glm/glm.hpp>

class SpatialGrid {
private:
    const int worldWidth, worldLength, cellSize;
    const int cellCountX, cellCountZ;
    const int worldOffsetX, worldOffsetZ;

    std::unordered_map<entt::entity, std::vector<entt::entity>*> entityIndex;
    std::vector<std::vector<entt::entity>> cells;
    std::unordered_set<entt::entity> dirtySet;

public:
    void getVisible(glm::vec3 camPos, std::vector<glm::vec3> frustomPoints,
        FrustumIntersection frustumTester, std::vector<entt::entity> dest);
};