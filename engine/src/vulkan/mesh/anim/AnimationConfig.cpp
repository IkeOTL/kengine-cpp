#include <kengine/vulkan/mesh/anim/AnimationConfig.hpp>
#include <functional>
#include <string>

namespace ke {
    size_t AnimationConfig::hashCode() const noexcept {
        std::hash<std::string> strHasher;

        size_t hash = 7;
        hash = 79 * hash + strHasher(animationSet);
        hash = 79 * hash + strHasher(animationName);

        return hash;
    }

    bool AnimationConfig::operator==(const AnimationConfig& other) const {
        if (this == &other)
            return true;

        if (this->animationSet != other.animationSet)
            return false;

        return this->animationName == other.animationName;
    }

    bool AnimationConfig::operator!=(const AnimationConfig& other) const {
        return !(*this == other);
    }
} // namespace ke