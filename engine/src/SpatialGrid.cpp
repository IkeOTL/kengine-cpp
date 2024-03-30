#include <kengine/SpatialGrid.hpp>
#include <kengine/util/MatUtils.hpp>
#include <kengine/Math.hpp>

glm::vec3 SpatialGrid::intersectPoint(const glm::vec3& start, const glm::vec3& end) {
    // can't intersect with even ground plane
    if ((start.y > 0 && end.y > 0) || (start.y < 0 && end.y < 0))
        return glm::vec3(end.x, 0, end.z);

    auto factor = start.y / (start.y - end.y);

    return glm::mix(start, end, factor);
}


void SpatialGrid::getVisible(glm::vec3 camPos, std::vector<glm::vec3> frustomPoints,
    FrustumIntersection& frustumTester, std::vector<entt::entity> dest) {
    using namespace matutils;
    auto topLeft = intersectPoint(frustomPoints[FrustumCorner::CORNER_NXPYNZ], frustomPoints[FrustumCorner::CORNER_NXPYPZ]);
    auto bottomLeft = intersectPoint(frustomPoints[FrustumCorner::CORNER_NXNYNZ], frustomPoints[FrustumCorner::CORNER_NXNYPZ]);
    auto bottomRight = intersectPoint(frustomPoints[FrustumCorner::CORNER_PXNYNZ], frustomPoints[FrustumCorner::CORNER_PXNYPZ]);
    auto topRight = intersectPoint(frustomPoints[FrustumCorner::CORNER_PXPYNZ], frustomPoints[FrustumCorner::CORNER_PXPYPZ]);

    auto totMin = glm::min(topLeft, glm::min(topRight, glm::min(bottomLeft, glm::min(bottomRight, camPos))));
    auto totMax = glm::max(topLeft, glm::max(topRight, glm::max(bottomLeft, glm::max(bottomRight, camPos))));

    auto startCellX = (int)(std::floor(totMin.x - worldOffsetX) / cellSize);
    auto startCellZ = (int)(std::floor(totMin.z - worldOffsetZ) / cellSize);
    auto endCellX = (int)(std::floor(totMax.x - worldOffsetX) / cellSize);
    auto endCellZ = (int)(std::floor(totMax.z - worldOffsetZ) / cellSize);

    startCellX = math::max(0, math::min(startCellX, cellCountX - 1));
    startCellZ = math::max(0, math::min(startCellZ, cellCountZ - 1));

    endCellX = math::max(0, math::min(endCellX, cellCountX - 1));
    endCellZ = math::max(0, math::min(endCellZ, cellCountZ - 1));
}