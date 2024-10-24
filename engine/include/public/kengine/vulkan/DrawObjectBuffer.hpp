#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include "VulkanContext.hpp"
#include "RenderContext.hpp"

namespace ke {
    struct DrawObject {
        glm::mat4 transform;
        glm::vec4 boundingSphere;
        uint32_t materialId;
        uint32_t padding[3];
    };

    class DrawObjectBuffer {
    public:
        static int alignedFrameSize(VulkanContext& vkCxt);
        static int frameSize();
    };
} // namespace ke