#include <kengine/Engine.hpp>
#include <glm/glm.hpp>
#include <kengine/vulkan/renderpass/DeferredPbrRenderPass.hpp>
#include <kengine/math.hpp>
#include <kengine/vulkan/pipelines/DeferredOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedOffscreenPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/DeferredCompositionPbrPipeline.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/SkinnedCascadeShadowMapPipeline.hpp>
#include <kengine/vulkan/pipelines/DrawCullingPipeline.hpp>

int main() {
    glm::mat4 Proj = glm::mat4();
    auto v0 = glm::normalize(glm::vec4(1, 1, 1, 0));
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
            passes.emplace_back(std::move(rp));

            return passes;
        },
        [](VulkanContext& vkCtx, std::vector<std::unique_ptr<RenderPass>>& rp) {
            auto pc = std::make_unique<PipelineCache>();

            auto pass0 = std::make_unique<DeferredOffscreenPbrPipeline>();
            pass0->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(pass0));

            auto skinned = std::make_unique<SkinnedOffscreenPbrPipeline>();
            skinned->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(skinned));

            auto pass1 = std::make_unique<DeferredCompositionPbrPipeline>();
            pass1->init(vkCtx, rp[0].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(pass1));

            auto shadowPass = std::make_unique<CascadeShadowMapPipeline>();
            shadowPass->init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });
            pc->addPipeline(std::move(shadowPass));

            auto skinnedShadowPass = std::make_unique<SkinnedCascadeShadowMapPipeline>();
            skinnedShadowPass->init(vkCtx, rp[1].get(), vkCtx.getDescSetLayoutCache(), glm::vec2{ 4096 , 4096 });
            pc->addPipeline(std::move(skinnedShadowPass));

            auto culling = std::make_unique<DrawCullingPipeline>();
            culling->init(vkCtx, nullptr, vkCtx.getDescSetLayoutCache(), glm::vec2{});
            pc->addPipeline(std::move(culling));

            return pc;
        },
        [](VulkanContext& vkCxt, Swapchain& swapchain, std::vector<std::unique_ptr<RenderPass>>& renderPasses) {
            std::vector<std::unique_ptr<RenderPass>> passes;

        }
    );

    engine.run();
    return 0;
}