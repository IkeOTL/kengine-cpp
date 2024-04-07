#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>


template <typename T, typename R, typename S>
class BoneTrack {
private:
    const uint32_t boneId;
    const std::string name;

    const std::vector<float> times;

    const std::vector<T> translations;
    const std::vector<R> rotations;
    const std::vector<S> scales;

public:
    BoneTrack(uint32_t boneId, std::string name,
        std::vector<T>&& translations,
        std::vector<R>&& rotations,
        std::vector<S>&& scales)
        : boneId(boneId), name(name),
        translations(std::move(translations)),
        rotations(std::move(rotations)),
        scales(std::move(scales))
    {}

    uint32_t getBoneId() const {
        return boneId;
    }

    const std::vector<float>& getTimes() const {
        return times;
    }

    uint32_t getFrameCount() const {
        return times.size();
    }

    float getStartTime() const {
        return times[0];
    }

    float getEndTime() const {
        return times[getFrameCount() - 1];
    }

    float getDuration() const {
        return getEndTime() - getStartTime();
    }

    const T& getTranslation(int i) const {
        return translations[i];
    }

    const R& getRotation(int i) const {
        return rotations[i];
    }

    const S& getScale(int i) const {
        return scales[i];
    }
};
