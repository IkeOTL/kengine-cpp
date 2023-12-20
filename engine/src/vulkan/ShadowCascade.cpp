#include <kengine/vulkan/ShadowCascade.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/Math.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

void ShadowCascade::updateViewProj(const glm::mat4& invCam, float camNear, const glm::vec3& lightDir,
    float lastSplitDist, float splitDist, float clipRange) {
    glm::vec3 frustumCorners[8] = {
           math::transformProject(invCam, glm::vec3(-1.0f, 1.0f, 0.0f)),
           math::transformProject(invCam, glm::vec3(1.0f, 1.0f, 0.0f)),
           math::transformProject(invCam, glm::vec3(1.0f, -1.0f, 0.0f)),
           math::transformProject(invCam, glm::vec3(-1.0f, -1.0f, 0.0f)),
           math::transformProject(invCam, glm::vec3(-1.0f, 1.0f, 1.0f)),
           math::transformProject(invCam, glm::vec3(1.0f, 1.0f, 1.0f)),
           math::transformProject(invCam, glm::vec3(1.0f, -1.0f, 1.0f)),
           math::transformProject(invCam, glm::vec3(-1.0f, -1.0f, 1.0f))
    };

    for (auto i = 0; i < 4; i++) {
        auto& cI = frustumCorners[i];
        auto& c4 = frustumCorners[i + 4];

        auto dist = c4 - cI;

        c4 = splitDist * dist + cI;
        cI += lastSplitDist * dist;
    }

    glm::vec3 frustumCenter{};
    for (auto i = 0; i < 8; i++)
        frustumCenter += frustumCorners[i];
    frustumCenter *= 0.125f;

    auto radius = 0.0f;
    for (auto i = 0; i < 8; i++) {
        auto diff = frustumCorners[i] - frustumCenter;
        auto distanceSq = glm::dot(diff, diff);
        radius = std::fmaxf(radius, distanceSq);
    }
    radius = std::ceilf(std::sqrtf(radius) * 16.0f) / 16.0f;
}

size_t ShadowCascadeData::alignedFrameSize(VulkanContext& vkCxt) {
    return vkCxt.alignUboFrame(size());
}

size_t ShadowCascadeData::size() {
    return(4 * sizeof(float)) + (ShadowCascadeData::SHADOW_CASCADE_COUNT * 16 * sizeof(float)) + (3 * sizeof(float));
}
