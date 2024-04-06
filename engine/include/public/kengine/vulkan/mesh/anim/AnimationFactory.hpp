#pragma once
#include <kengine/vulkan/mesh/anim/Animation.hpp>
#include <memory>

class VulkanContext;
class AssetIO;
class AnimationConfig;
class Animation;

class AnimationFactory {
private:
    VulkanContext& vkContext;
    AssetIO& assetIo;

public:
    AnimationFactory(VulkanContext& vkContext, AssetIO& assetIo)
        : vkContext(vkContext), assetIo(assetIo) {}

    virtual std::unique_ptr<Animation> loadAnimation(const AnimationConfig& config) = 0;

private:


};