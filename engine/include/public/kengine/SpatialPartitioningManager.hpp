#pragma once
#include <kengine/SpatialGrid.hpp>
#include <memory>

class SpatialPartitioningManager {
private:
    std::unique_ptr<SpatialGrid> spatialGrid;

public:
    SpatialGrid* getSpatialGrid() {
        return spatialGrid.get();
    }

    void setSpatialGrid(std::unique_ptr<SpatialGrid>&& g) {
        spatialGrid = std::move(g);
    }
};