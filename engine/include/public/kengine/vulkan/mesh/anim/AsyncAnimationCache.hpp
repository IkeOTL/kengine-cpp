#pragma once
#include <kengine/vulkan/AsyncAssetCache.hpp>
#include <kengine/vulkan/mesh/anim/Animation.hpp>
#include <kengine/vulkan/mesh/anim/AnimationConfig.hpp>

class Model;
class AnimationFactory;
class ExecutorService;

class AsyncAnimationCache : public AsyncAssetCache<Animation, AnimationConfig> {
private:
    AnimationFactory& factory;

protected:
    std::unique_ptr<Animation> create(std::shared_ptr<AnimationConfig> keyObj) override;

public:
    AsyncAnimationCache(AnimationFactory& factory, ExecutorService& workerPool)
        : AsyncAssetCache(workerPool), factory(factory) {}

};