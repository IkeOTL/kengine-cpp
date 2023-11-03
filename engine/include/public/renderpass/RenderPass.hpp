#pragma once
#include <vulkan/vulkan_core.h>
#include <type_traits>
#include <memory>
#include <vector>
#include <ColorFormatAndSpace.hpp>

struct RenderPassContext {
    const int renderPassIndex;
    const int frameBufferIndex;
    const VkCommandBuffer cmd;
    const glm::ivec2 cmd;
};

class RenderTarget;

template <typename RT>
class RenderPass {
    static_assert(std::is_base_of<RenderTarget, RT>::value, "Generic must derive from RenderTarget");

private:
    const VkDevice vkDevice;
    const ColorFormatAndSpace ColorFormatAndSpace;

    VkRenderPass vkRenderPass;
    std::vector<RT> renderTargets;

    virtual RT* createRenderTarget() = 0;

public:

};