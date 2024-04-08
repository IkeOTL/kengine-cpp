#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

class Transform;

class BoneTrack {
protected:
    const uint32_t boneId;
    const std::string name;

    const std::vector<float> times;
    const std::vector<glm::vec3> translation;
    const std::vector<glm::quat> rotation;
    const std::vector<glm::vec3> scale;

public:
    BoneTrack(uint32_t boneId, std::string name, std::vector<float>&& times,
        std::vector<glm::vec3>&& translation, std::vector<glm::quat>&& rotation, std::vector<glm::vec3>&& scale)
        : boneId(boneId), name(name), times(std::move(times)),
        translation(std::move(translation)), rotation(std::move(rotation)), scale(std::move(scale))
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
};