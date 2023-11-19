#include <renderpass/RenderPass.hpp>
#include <glm/vec2.hpp>

void RenderPass::init() {
    vkRenderPass = createVkRenderPass();
}

const RenderTarget* RenderPass::getRenderTarget(size_t renderTargetIndex) const {
    if (renderTargetIndex >= renderTargets.size())
        throw std::runtime_error("Render target index out of bounds.");

    return renderTargets[renderTargetIndex].get();
}