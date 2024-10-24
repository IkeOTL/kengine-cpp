#pragma once
#include <kengine/Hashable.hpp>
#include <string>
#include <memory>

namespace ke {
    class AnimationConfig : Hashable {
    private:
        const std::string animationSet;
        const std::string animationName;

    public:
        AnimationConfig(std::string animationSet, std::string animationName) :
            animationSet(animationSet), animationName(animationName) {}

        inline static std::shared_ptr<AnimationConfig> create(std::string animationSet, std::string animationName) {
            return std::make_shared<AnimationConfig>(animationSet, animationName);
        }

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
} // namespace ke