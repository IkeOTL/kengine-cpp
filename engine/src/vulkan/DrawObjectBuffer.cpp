#include <kengine/vulkan/DrawObjectBuffer.hpp>

int DrawObjectBuffer::alignedFrameSize(VulkanContext& vkCxt) {
    return (int)vkCxt.alignSsboFrame(RenderContext::MAX_INSTANCES * sizeof(DrawObject));
}

int DrawObjectBuffer::frameSize() {
    return RenderContext::MAX_INSTANCES * sizeof(DrawObject);
}