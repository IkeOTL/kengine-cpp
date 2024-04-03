#include <kengine/vulkan/ShadowCascade.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/util/MatUtils.hpp>
#include <glm/gtc/matrix_transform.hpp>

void ShadowCascade::updateViewProj(const glm::mat4& invCam, float camNear, const glm::vec3& lightDir,
    float lastSplitDist, float splitDist, float clipRange) {

    glm::vec3 baseCorners[8] = {
        glm::vec3(-1.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f)
    };

    glm::vec3 frustumCorners[8] = {
           matutils::transformProject(invCam, baseCorners[0]),
           matutils::transformProject(invCam, baseCorners[1]),
           matutils::transformProject(invCam, baseCorners[2]),
           matutils::transformProject(invCam, baseCorners[3]),
           matutils::transformProject(invCam, baseCorners[4]),
           matutils::transformProject(invCam, baseCorners[5]),
           matutils::transformProject(invCam, baseCorners[6]),
           matutils::transformProject(invCam, baseCorners[7])
    };

    for (auto i = 0; i < 4; i++) {
      /*  auto& cI = frustumCorners[i];
        auto& c4 = frustumCorners[i + 4];

        auto dist = c4 - cI;

        c4 = splitDist * dist + cI;
        cI += lastSplitDist * dist;*/
        glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
        frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
        frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
    }

    glm::vec3 frustumCenter(0.0f);
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
     
    auto maxExtents = glm::vec3(radius);
    auto minExtents = -maxExtents;

    auto lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
    auto lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
    this->viewProj = lightOrthoMatrix * lightViewMatrix;

    this->splitDepth = -(camNear + splitDist * clipRange);
}

ShadowCascade& ShadowCascadeData::getCascade(int i) {
    return cascades[i];
}

void ShadowCascadeData::uploadShadowPass(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, int frameIdx) {
    glm::mat4 cascadeViewProjs[ShadowCascadeData::SHADOW_CASCADE_COUNT];

    for (auto i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++)
        memcpy(&cascadeViewProjs[i], &cascades[i].getViewProj(), sizeof(glm::mat4));

    auto size = ShadowCascadeData::SHADOW_CASCADE_COUNT * sizeof(glm::mat4);
    auto pos = gpuBuffer.getFrameOffset(frameIdx);

    auto buf = static_cast<unsigned char*>(gpuBuffer.getGpuBuffer().data());
    memcpy(buf + pos, &cascadeViewProjs[0], size);
    gpuBuffer.getGpuBuffer().flush(pos, size);
}

void ShadowCascadeData::uploadCompositionPass(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, int frameIdx) {
    struct ToUpload {
        glm::mat4 viewProj[ShadowCascadeData::SHADOW_CASCADE_COUNT];
        glm::vec4 splits;
        glm::vec3 lightDir;
    };

    ToUpload data{};
    {
        for (auto i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++)
            data.viewProj[i] = cascades[i].getViewProj();
        //memcpy(&data.viewProj[i], &cascades[i].getViewProj(), sizeof(glm::mat4));

        for (auto i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++)
            data.splits[i] = cascades[i].getSplitDepth();

        data.lightDir = lightDir;
        //memcpy(&data.lightDir, &lightDir, sizeof(glm::vec3));
    }

    auto size = sizeof(ToUpload);
    auto pos = gpuBuffer.getFrameOffset(frameIdx);

    auto buf = static_cast<unsigned char*>(gpuBuffer.getGpuBuffer().data());
    memcpy(buf + pos, &data, size);
    gpuBuffer.getGpuBuffer().flush(pos, size);
}

size_t ShadowCascadeData::alignedFrameSize(VulkanContext& vkCxt) {
    return vkCxt.alignUboFrame(size());
}

size_t ShadowCascadeData::size() {
    return(ShadowCascadeData::SHADOW_CASCADE_COUNT * sizeof(float))
        + (ShadowCascadeData::SHADOW_CASCADE_COUNT * 16 * sizeof(float))
        + (3 * sizeof(float));
}
