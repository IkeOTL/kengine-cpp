#include <kengine/vulkan/DrawObjectBuffer.hpp>

namespace ke {
    int DrawObjectBuffer::alignedFrameSize(VulkanContext& vkCxt) {
        return (int)vkCxt.alignSsboFrame(RenderContext::MAX_INSTANCES * sizeof(DrawObject));
    }

    int DrawObjectBuffer::frameSize() {
        return RenderContext::MAX_INSTANCES * sizeof(DrawObject);
    }
} // namespace ke