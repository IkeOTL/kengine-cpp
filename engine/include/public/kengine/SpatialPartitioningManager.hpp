#pragma once
#include <kengine/SpatialGrid.hpp>
#include <memory>

namespace ke {
    class SpatialPartitioningManager {
    private:
        std::unique_ptr<SpatialGrid> spatialGrid;

    public:
        inline static std::unique_ptr<SpatialPartitioningManager> create() {
            return std::make_unique<SpatialPartitioningManager>();
        }

        SpatialGrid* getSpatialGrid() {
            return spatialGrid.get();
        }

        void setSpatialGrid(std::unique_ptr<SpatialGrid>&& g) {
            spatialGrid = std::move(g);
        }
    };
} // namespace ke