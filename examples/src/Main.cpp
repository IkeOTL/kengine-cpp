#include <kengine/Engine.hpp>
#include <glm/glm.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/math.hpp>

int main() {
    glm::mat4 Proj = glm::mat4();
    auto v0 = glm::normalize(glm::vec4(1,1,1,0));
    auto v = glm::vec4(1, 1, 1, 0) * .5f;
    auto v1 = glm::normalize(glm::vec4(1, 1, 1, 22));
    glm::normalize(glm::vec4());

    auto m = glm::mat4();
    m[0][3] = 666;
    auto p = math::frustumPlane(m, math::PLANE_NX);

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