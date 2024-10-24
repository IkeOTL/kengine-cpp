#include <kengine/vulkan/mesh/anim/AsyncAnimationCache.hpp>
#include <kengine/vulkan/mesh/anim/AnimationFactory.hpp>
#include <kengine/vulkan/mesh/anim/AnimationConfig.hpp>

namespace ke {
    std::unique_ptr<Animation> AsyncAnimationCache::create(std::shared_ptr<AnimationConfig> keyObj) {
        return factory.loadAnimation(*keyObj);
    }
} // namespace ke