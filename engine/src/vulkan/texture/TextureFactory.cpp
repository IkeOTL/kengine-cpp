#include <kengine/vulkan/texture/TextureFactory.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb_image.h>

std::unique_ptr<Texture2d> TextureFactory::loadTexture(const TextureConfig& config) {
    auto& key = config.getTextureKey();

    auto assetData = assetIo.load(key);

    int width, height, channels;
    auto imageData = stbi_load_from_memory(
        assetData->data(), assetData->length(),
        &width, &height, &channels, config.getChannels()
    );

    if (!imageData)
        throw std::runtime_error("Failed to load image.");

    auto texture = std::make_unique<Texture2d>(
        vkContext, imageData, width, height, config.getFormat(),
        config.getImageType(), config.getImageViewType(),
        config.getChannels(), config.getDstStageMask(), config.getDstAccessMask(),
        config.hasMipmaps());

    stbi_image_free(imageData);

    return texture;
}
