#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/texture/TextureConfig.hpp>
#include <kengine/vulkan/texture/Texture2D.hpp>
#include <kengine/io/AssetIO.hpp>

namespace ke {
    class TextureFactory {
    private:
        VulkanContext& vkContext;
        AssetIO& assetIo;

    public:
        TextureFactory(VulkanContext& vkContext, AssetIO& assetIo)
            : vkContext(vkContext),
              assetIo(assetIo) {};

        inline static std::unique_ptr<TextureFactory> create(VulkanContext& vkContext, AssetIO& assetIo) {
            return std::make_unique<TextureFactory>(vkContext, assetIo);
        }

        std::unique_ptr<Texture2d> loadTexture(const TextureConfig& config);
    };
} // namespace ke