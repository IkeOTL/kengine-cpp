#include <kengine/SpatialGrid.hpp>
#include <kengine/util/MatUtils.hpp>
#include <kengine/Math.hpp>
#include <kengine/Logger.hpp>


SpatialGrid::SpatialGrid(uint32_t worldWidth, uint32_t worldLength, uint32_t cellSize)
    : worldWidth(worldWidth), worldLength(worldLength), cellSize(cellSize),
    cellCountX(worldWidth / cellSize), cellCountZ(worldLength / cellSize),
    worldOffsetX(-static_cast<int32_t>(worldWidth) / 2), worldOffsetZ(-static_cast<int32_t>(worldLength) / 2)
{
    if (worldWidth % 2 != 0 || worldLength % 2 != 0)
        throw std::runtime_error("Grid dimensions must be even.");

    if (worldWidth % cellSize != 0 || worldLength % cellSize != 0)
        throw std::runtime_error("Grid dimensions a multiple of cell size.");

    cells.resize(cellCountX * cellCountZ);
    for (auto z = 0; z < cellCountZ; z++)
        for (auto x = 0; x < cellCountX; x++)
            cells[z * cellCountX + x].reserve(64);
}

void SpatialGrid::setDirty(const entt::entity entity) {
    dirtySet.insert(entity);
}

std::function<void()> SpatialGrid::createCb(const entt::entity eId) {
    return [this, eId]() { this->setDirty(eId); };
}

glm::vec3 SpatialGrid::intersectPoint(const glm::vec3& start, const glm::vec3& end) {
    // ray never intersects with floor plane
    // lets default the intersection happening at the end of the ray
    if ((start.y > 0 && end.y > 0) || (start.y < 0 && end.y < 0))
        return glm::vec3(end.x, 0, end.z);

    auto factor = start.y / (start.y - end.y);

    return glm::mix(start, end, factor);
}

/// <summary>
/// gets cells visible to frustum and then grab all the entity ids for visible cells and adds them to the output list
/// </summary>
void SpatialGrid::getVisible(const glm::vec3& camPos, const std::array<glm::vec3, 8>& frustomPoints,
    const FrustumIntersection& frustumTester, std::vector<entt::entity>& dest) {
    using namespace matutils;
    auto topLeft = intersectPoint(frustomPoints[FrustumCorner::CORNER_NXPYNZ], frustomPoints[FrustumCorner::CORNER_NXPYPZ]);
    auto bottomLeft = intersectPoint(frustomPoints[FrustumCorner::CORNER_NXNYNZ], frustomPoints[FrustumCorner::CORNER_NXNYPZ]);
    auto bottomRight = intersectPoint(frustomPoints[FrustumCorner::CORNER_PXNYNZ], frustomPoints[FrustumCorner::CORNER_PXNYPZ]);
    auto topRight = intersectPoint(frustomPoints[FrustumCorner::CORNER_PXPYNZ], frustomPoints[FrustumCorner::CORNER_PXPYPZ]);

    auto totMin = glm::min(topLeft, glm::min(topRight, glm::min(bottomLeft, glm::min(bottomRight, camPos))));
    auto totMax = glm::max(topLeft, glm::max(topRight, glm::max(bottomLeft, glm::max(bottomRight, camPos))));

    auto startCellX = static_cast<int32_t>(std::floor(totMin.x - worldOffsetX) / cellSize);
    auto startCellZ = static_cast<int32_t>(std::floor(totMin.z - worldOffsetZ) / cellSize);
    auto endCellX = static_cast<int32_t>(std::floor(totMax.x - worldOffsetX) / cellSize);
    auto endCellZ = static_cast<int32_t>(std::floor(totMax.z - worldOffsetZ) / cellSize);

    startCellX = math::max(0, math::min(startCellX, cellCountX - 1));
    startCellZ = math::max(0, math::min(startCellZ, cellCountZ - 1));

    endCellX = math::max(0, math::min(endCellX, cellCountX - 1));
    endCellZ = math::max(0, math::min(endCellZ, cellCountZ - 1));

    std::unordered_set<entt::entity> set;

    for (auto z = startCellZ; z <= endCellZ; z++) {
        for (auto x = startCellX; x <= endCellX; x++) {
            int32_t minWorldX = cellSize * x + worldOffsetX;
            int32_t minWorldZ = cellSize * z + worldOffsetZ;
            int32_t maxWorldX = minWorldX + cellSize;
            int32_t maxWorldZ = minWorldZ + cellSize;

            // maybe use entity heights here?
            auto hit = frustumTester.testAab(minWorldX, 0, minWorldZ, maxWorldX, 0, maxWorldZ);

            if (!hit)
                continue;

            {
                // read lock
                std::shared_lock<std::shared_mutex> lock(this->lock);

                auto cellIndex = z * cellCountX + x;
                auto& cell = cells[cellIndex];

                for (int i = 0; i < cell.size(); i++)
                    set.insert(cell[i]);
            }
        }
    }

    for (auto& e : set)
        dest.push_back(e);
}


void SpatialGrid::updateEntity(entt::entity entityId, const glm::mat4& xform, const Aabb& aabb) {
    removeEntity(entityId);
    addEntity(entityId, xform, aabb);
}

void SpatialGrid::addEntity(entt::entity entityId, const glm::mat4& xform, const Aabb& aabb) {
    glm::vec3 min;
    glm::vec3 max;
    aabb.getMinMax(min, max);
    matutils::transformAab(xform, min, max, min, max);


    KE_LOG_INFO(std::format("min.x: {}, min.y: {}, min.z: {}", min.x, min.y, min.z));
    KE_LOG_INFO(std::format("max.x: {}, max.y: {}, max.z: {}", max.x, max.y, max.z));

    // Adjust for the world's offset and then compute the row and column
    auto startCellX = static_cast<int32_t>(std::floor(min.x - worldOffsetX) / cellSize);
    auto startCellZ = static_cast<int32_t>(std::floor(min.z - worldOffsetZ) / cellSize);
    auto endCellX = static_cast<int32_t>(std::floor(max.x - worldOffsetX) / cellSize);
    auto endCellZ = static_cast<int32_t>(std::floor(max.z - worldOffsetZ) / cellSize);

    startCellX = math::max(0, math::min(startCellX, cellCountX - 1));
    startCellZ = math::max(0, math::min(startCellZ, cellCountZ - 1));

    endCellX = math::max(0, math::min(endCellX, cellCountX - 1));
    endCellZ = math::max(0, math::min(endCellZ, cellCountZ - 1));

    KE_LOG_INFO(std::format("endCellX: {}, endCellZ: {}", endCellX, endCellZ));

    auto& index = entityIndex[entityId];

    // reset index list
    index.fill(-1);

    auto curIdx = 0;
    for (auto z = startCellZ; z <= endCellZ && curIdx < MAX_CELLS_PER_ENTITY; z++) {
        for (auto x = startCellX; x <= endCellX; x++) {
            if (curIdx == MAX_CELLS_PER_ENTITY)
                break;


            // read/write lock
            std::lock_guard<std::shared_mutex> lock(this->lock);

            auto cellIndex = z * cellCountX + x;
            KE_LOG_INFO(std::format("cellindex: {}", cellIndex));
            cells[cellIndex].push_back(entityId);

            // add to index for fast access in opposite direction            
            index[curIdx++] = cellIndex;
        }

        if (curIdx == MAX_CELLS_PER_ENTITY)
            break;
    }
    KE_LOG_INFO("");
}

void SpatialGrid::removeEntity(entt::entity entityId) {
    // read/write lock
    std::lock_guard<std::shared_mutex> lock(this->lock);

    auto it = entityIndex.find(entityId);
    if (it == entityIndex.end())
        return;

    auto idxBag = std::move(it->second);
    entityIndex.erase(it);

    for (auto& c : idxBag) {
        if (c == -1)
            return;

        auto& cell = cells[c];
        cell.erase(std::remove(cell.begin(), cell.end(), entityId), cell.end());
    }
}

/// <summary>
/// todo: parallelize
/// while this is executing adding and removing from dirty set shoudl not be happening
/// if we change to a reentrant lock it might be possible thought
/// </summary>
void SpatialGrid::processDirtyEntities(std::function<SpatialGridUpdate(entt::entity)> func) {
    for (auto& e : dirtySet) {
        auto update = func(e);
        updateEntity(update.entity, update.transform, update.bounds);
    }

    dirtySet.clear();
}
