#pragma once
#include <kengine/Hashable.hpp>
#include <string>

class AnimationConfig : Hashable {
private:
    const std::string animationSet;
    const std::string animationName;

public:
    AnimationConfig(std::string animationSet, std::string animationName) :
        animationSet(animationSet), animationName(animationName) {}

    std::string getAnimationSet() const {
        return animationSet;
    }

    std::string getAnimationName() const {
        return animationName;
    }

    size_t hashCode() const noexcept override;
    bool operator==(const AnimationConfig& other) const;
    bool operator!=(const AnimationConfig& other) const;
};