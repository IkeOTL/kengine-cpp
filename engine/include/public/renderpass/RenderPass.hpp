#pragma once
#include <VulkanInclude.hpp>
#include <type_traits>
#include <memory>
#include <vector>
#include <ColorFormatAndSpace.hpp>
#include <glm/vec2.hpp>

class RenderTarget;

template <typename RT>
class RenderPass {
    static_assert(std::is_base_of<RenderTarget, RT>::value, "Generic must derive from RenderTarget");

public:
    struct RenderPassContext {
        const int renderPassIndex;
        const int frameBufferIndex;
        const VkCommandBuffer cmd;
        const glm::ivec2 extents;
    };

    RenderPass();

    void vkDispose() {
        for (auto& rt : renderTargets)
            rt->vkDispose();
    }

private:
    const VkDevice vkDevice;
    const ColorFormatAndSpace ColorFormatAndSpace;

    VkRenderPass vkRenderPass;
    std::vector<std::shared_ptr<RT>> renderTargets;

    void addRenderTarget(std::shared_ptr<RT> rt) {
        renderTargets.push_back(rt)
    }

    void freeRenderTargets() {
        for (auto& rt : renderTargets)
            rt->vkDispose();

        renderTargets.clear();
    }

    virtual std::shared_ptr<RT> createRenderTarget() = 0;
    virtual void begin(RenderPassContext& cxt) = 0;
    virtual void end(RenderPassContext& cxt) = 0;

};