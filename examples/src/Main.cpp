#include <kengine/Engine.hpp>
#include <glm/glm.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>

int main() {
    glm::mat4 Proj = glm::mat4();

    Engine engine(
        [](VkDevice vkDevice, ColorFormatAndSpace& cfs) {
            std::vector<std::unique_ptr<RenderPass>> passes;

            auto rp = std::make_unique<DeferredPbrRenderPass>(vkDevice, cfs);
            rp->init();
            passes.emplace_back(std::move(rp));

            return passes;
        },
        [](VulkanContext& vkCxt, Swapchain& swapchain, std::vector<std::unique_ptr<RenderPass>>& renderPasses) {
            std::vector<std::unique_ptr<RenderPass>> passes;

        }
        );

    engine.run();
    return 0;
}