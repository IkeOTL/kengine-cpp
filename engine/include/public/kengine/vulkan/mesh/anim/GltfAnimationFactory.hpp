#pragma once
#include <kengine/vulkan/mesh/anim/AnimationFactory.hpp>
#include <thirdparty/tiny_gltf.h>
#include <thread>

class VulkanContext;
class AssetIO;
class AnimationConfig;
class Animation;

class GltfAnimationFactory : public AnimationFactory {
private:
    static thread_local tinygltf::TinyGLTF gltfLoader;

    VulkanContext& vkContext;
    AssetIO& assetIo;

public:
    GltfAnimationFactory(VulkanContext& vkContext, AssetIO& assetIo)
        : vkContext(vkContext), assetIo(assetIo) {}

    std::unique_ptr<Animation> loadAnimation(const AnimationConfig& config) override;
};