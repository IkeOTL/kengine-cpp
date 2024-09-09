#include <kengine/vulkan/SceneData.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/Camera.hpp>
#include <glm/gtc/matrix_transform.hpp>


void SceneData::upload(VulkanContext& vkCxt, CachedGpuBuffer& buffer, float time, float alpha, size_t frameIndex) {
    struct ToUpload {
        glm::mat4 proj;
        glm::mat4 view;
        glm::vec4 lightDir;
    };

    auto camera = cameraController.getCamera();
    ToUpload data{};
    data.proj = camera->getProjectionMatrix();
    //camera->getViewMatrix(data.view);
    camera->getIntegratedViewMatrix(alpha, data.view);
    data.lightDir = { lightDir, time };

    auto startPos = buffer.getFrameOffset(frameIndex);
    memcpy(static_cast<char*>(buffer.getGpuBuffer().data()) + startPos, &data, sizeof(ToUpload));
    buffer.getGpuBuffer().flush(startPos, sizeof(ToUpload));
}

size_t SceneData::alignedFrameSize(VulkanContext& vkCxt) {
    return vkCxt.alignUboFrame(size());
}

size_t SceneData::size() {
    return Camera::size() + (4 * sizeof(float)) + sizeof(float);
}