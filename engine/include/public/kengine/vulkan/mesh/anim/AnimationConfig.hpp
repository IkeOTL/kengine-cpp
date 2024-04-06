#pragma once
#include <kengine/Hashable.hpp>

class AnimationConfig : Hashable {
private:
    const std::string animationSet;
    const std::string animationName;

public:
    AnimationConfig(std::string animationSet, std::string animationName) :
        animationSet(animationSet), animationName(animationName) {}

    std::string getAnimationSet() {
        return animationSet;
    }

    std::string getAnimationName() {
        return animationName;
    }

    size_t hashCode() const noexcept override;
    bool operator==(const AnimationConfig& other) const;
    bool operator!=(const AnimationConfig& other) const;
};