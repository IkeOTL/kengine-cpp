#pragma once
#include <kengine/vulkan/mesh/anim/BoneTrack.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

template<typename T, typename R, typename S>
class Animation {
private:
    const std::string name;
    const std::vector<BoneTrack<T, R, S>> tracks;
    std::vector<std::vector<uint32_t>> sampleIndices;

    float startTime = 0, endTime = 0;

    const float ticksPerSecond;
    float duration;

public:
    Animation(std::string name, float ticksPerSecond, float duration, std::vector<BoneTrack<T, R, S>>&& tracks);

    float apply(Skeleton& skeleton, float time, bool loop);

    float adjustTime(float inTime, bool looping);
    int getTargetFrameIdx(int boneIdx, float time, bool loop);
    void calculateMaxRange();
    void updateSamples();
};