#include <renderpass/DeferredPbrRenderPass.hpp>
#include <glm/vec2.hpp>

void DeferredPbrRenderPass::begin(RenderPassContext& cxt)
{
}

void DeferredPbrRenderPass::end(RenderPassContext& cxt)
{
}

std::unique_ptr<VkRenderPass> DeferredPbrRenderPass::createVkRenderPass() {
    return std::unique_ptr<VkRenderPass>();
}

std::unique_ptr<VmaImage::ImageAndView> DeferredPbrRenderPass::createDepthStencil(VmaAllocator vmaAllocator, glm::ivec2 extents) {
    VkImageView vkImageView;
    VkImage vkImage;
    VmaAllocation vmaImageAllocation;

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    //  imageCreateInfo.format = getColorFormatAndSpace().getColorFormat();
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    // create vkImageview
    // create vkImage

    auto imageAndView = std::make_unique<VmaImage::ImageAndView>(VmaImage::ImageAndView{
            std::make_shared<VmaImage>(vmaAllocator, vkImage, vmaImageAllocation),
            vkImageView
        });

    return imageAndView;
}

std::unique_ptr<RenderTarget> DeferredPbrRenderPass::createRenderTarget(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::ivec2& extents, const int renderTargetIndex)
{
    return std::unique_ptr<RenderTarget>();
}

void DeferredPbrRenderPass::createRenderTargets(VmaAllocator vmaAllocator, const std::vector<VkImageView>& sharedImageViews, const glm::ivec2& extents) {
}