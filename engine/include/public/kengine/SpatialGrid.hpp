#pragma once
#include <thirdparty/entt.hpp>

class SpatialGrid {
private:
    const int worldWidth, worldLength, cellSize;
    const  int cellCountX, cellCountZ;
    const int worldOffsetX, worldOffsetZ;

    std::unordered_map<entt::entity, std::vector<entt::entity>*> entityIndex;
    std::vector<std::vector<entt::entity>> cells;
    std::unordered_set < entt::entity> dirtySet;
public:

};