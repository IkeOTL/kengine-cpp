#include <renderpass/RenderPass.hpp>
#include <glm/vec2.hpp>

void RenderPass::init() {
    vkRenderPass = createVkRenderPass();
}