#include <kengine/vulkan/mesh/Mesh.hpp>
#include <glm/vec3.hpp>

void MeshData::calcBounds() {
    glm::vec3 min(9999999);
    glm::vec3 max(-9999999);

    for (auto& vert : vertices) {
        auto& pos = vert->getPosition();
        min = glm::min(pos, min);
        max = glm::max(pos, max);
    }

    bounds = Bounds::fromMinMax(min, max);
}