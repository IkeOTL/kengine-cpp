#pragma once
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

class ShadowCascade {
private:
    float splitDepth;
    glm::mat4 viewProj;

public:
    void updateViewProj(const glm::mat4& invCam, float camNear, const glm::vec3& lightDir,
        float lastSplitDist, float splitDist, float clipRange);

    float getSplitDepth() {
        return splitDepth;
    }

    glm::mat4 getViewProj() {
        return viewProj;
    }
};

class ShadowCascadeData {
private:
    glm::vec3 lightDir;
    std::vector<std::unique_ptr<ShadowCascade>> cascades;

public:
    const static uint32_t SHADOW_CASCADE_COUNT = 4;

    void addCascade(ShadowCascade&& c);
    ShadowCascade* getCascade(int i);
    void uploadShadowPass(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, size_t frameIndex);
    void uploadCompositionPass(VulkanContext& vkCxt, CachedGpuBuffer& gpuBuffer, size_t frameIndex);

    void setLightDir(glm::vec3 l) {
        lightDir = l;
    }

    static size_t alignedFrameSize(VulkanContext& vkCxt);
    static size_t size();
};