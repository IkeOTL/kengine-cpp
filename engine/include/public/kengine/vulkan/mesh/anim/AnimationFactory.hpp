#pragma once
#include <kengine/vulkan/mesh/anim/Animation.hpp>
#include <memory>

namespace ke {
    class AnimationConfig;

    class AnimationFactory {

    public:
        virtual ~AnimationFactory() = default;

        virtual std::unique_ptr<Animation> loadAnimation(const AnimationConfig& config) = 0;

    private:


    };
} // namespace ke