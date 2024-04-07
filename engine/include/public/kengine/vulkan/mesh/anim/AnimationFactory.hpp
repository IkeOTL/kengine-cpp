#pragma once
#include <kengine/vulkan/mesh/anim/Animation.hpp>
#include <memory>

class AnimationFactory {

public:
    virtual ~AnimationFactory() = default;

    virtual std::unique_ptr<Animation> loadAnimation(const AnimationConfig& config) = 0;

private:


};