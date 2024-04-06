#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

class BoneTrack {
private:
    const uint32_t boneId;
    const std::string name;

    const std::vector<float> times;

    const std::vector<glm::vec3> translations;
    const std::vector<glm::quat> rotations;
    const std::vector<glm::vec3> scales;

public:
    BoneTrack(uint32_t boneId, std::string name,
        std::vector<glm::vec3>&& translations,
        std::vector<glm::quat>&& rotations,
        std::vector<glm::vec3>&& scales)
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

    const glm::vec3 getTranslation(int i) const {
        return translations[i];
    }

    const glm::quat getRotation(int i) const {
        return rotations[i];
    }

    const glm::vec3 getScale(int i) const {
        return scales[i];
    }
};