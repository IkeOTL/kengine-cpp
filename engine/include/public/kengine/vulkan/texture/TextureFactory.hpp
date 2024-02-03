#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/texture/TextureConfig.hpp>
#include <kengine/vulkan/texture/Texture2D.hpp>
#include <kengine/io/AssetIO.hpp>

class TextureFactory {
private:
    VulkanContext& vkContext;
    AssetIO& assetIo;

public:
    TextureFactory(VulkanContext& vkContext, AssetIO& assetIo)
        : vkContext(vkContext), assetIo(assetIo) {};

    std::unique_ptr<Texture2d> loadTexture(TextureConfig config);
};