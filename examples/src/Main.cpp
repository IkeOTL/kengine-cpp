#include "Engine.hpp"
#include <glm/glm.hpp>
#include <renderpass/DeferredPbrRenderPass.hpp>

int main() {
    glm::mat4 Proj = glm::mat4();
    Engine engine([](VkDevice vkDevice, ColorFormatAndSpace& cfs) -> std::vector<std::unique_ptr<RenderPass>> {
        std::vector<std::unique_ptr<RenderPass>> passes;
        auto lol = std::make_unique<DeferredPbrRenderPass>(vkDevice, cfs);
        passes.emplace_back(std::move(lol));
        return passes;
        });
    engine.run();
    return 0;
}