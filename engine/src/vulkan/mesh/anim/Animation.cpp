#include <kengine/vulkan/mesh/anim/Animation.hpp>


Animation::Animation(std::string name, float ticksPerSecond, float duration, std::vector<BoneTrack>&& tracks)
    : name(name), ticksPerSecond(ticksPerSecond), duration(duration), tracks(std::move(tracks)) {
    calculateMaxRange();
    updateSamples(this->tracks);
}

float Animation::adjustTime(float inTime, bool looping) {
    if (!looping) {
        if (inTime < startTime)
            return startTime;

        if (inTime > endTime)
            return endTime;

        return inTime;
    }

    auto maxDuration = endTime - startTime;
    if (maxDuration <= 0)
        return 0;

    inTime = std::fmodf(inTime - startTime, maxDuration);
    if (inTime < 0.0f)
        inTime += maxDuration;

    return inTime + startTime;
}

int Animation::getTargetFrameIdx(int boneIdx, float time, bool loop) {
    auto& boneTrack = tracks[boneIdx];
    auto trackSize = boneTrack.getFrameCount();

    if (trackSize <= 1)
        return -1;

    auto& times = boneTrack.getTimes();

    if (!loop) {
        if (time <= times[0])
            return 0;

        if (time >= times[trackSize - 2])
            return trackSize - 2;
    }

    float startTime = times[0];
    float endTime = times[trackSize - 1];
    time = std::fmodf(time - startTime, endTime - startTime);
    if (time < 0.0f)
        time += endTime - startTime;
    time = time + startTime;

    auto& sampledFrames = sampleIndices[boneIdx];

    float duration = boneTrack.getDuration();
    int numSamples = static_cast<int>(ticksPerSecond) + static_cast<int>(duration * ticksPerSecond);
    float t = time / duration;

    auto index = static_cast<int>(t * numSamples);
    if (index >= sampledFrames.size())
        return -1;

    return sampledFrames[index];
}

void Animation::calculateMaxRange()
{
    startTime = 0.0f;
    endTime = 0.0f;
    auto startSet = false;
    auto endSet = false;
    for (auto i = 0; i < tracks.size(); ++i) {
        float trackStartTime = tracks[i].getStartTime();
        float trackEndTime = tracks[i].getEndTime();

        if (trackStartTime < startTime || !startSet) {
            startTime = trackStartTime;
            startSet = true;
        }

        if (trackEndTime > endTime || !endSet) {
            endTime = trackEndTime;
            endSet = true;
        }
    }
}

void Animation::updateSamples() {
    sampleIndices.clear();
    sampleIndices.resize(tracks.size());
    for (auto b = 0; b < tracks.size(); b++) {
        auto& boneTrack = tracks[b];
        uint32_t numFrames = boneTrack.getFrameCount();

        if (numFrames <= 1)
            continue;

        float trackDuration = boneTrack.getDuration();
        int numSamples = static_cast<int>(ticksPerSecond) + static_cast<int>(trackDuration * ticksPerSecond);
        auto& sampledFrames = sampleIndices[b];
        sampledFrames.reserve(numSamples);

        auto& times = boneTrack.getTimes();
        for (auto s = 0; s < numSamples; ++s) {
            float t = static_cast<float>(s) / static_cast<float>(numSamples - 1);
            float time = t * trackDuration + boneTrack.getStartTime();

            int frameIndex = 0;
            for (auto j = numFrames - 1; j >= 0; --j) {
                if (time >= times[j]) {
                    frameIndex = j;
                    if (frameIndex >= numFrames - 2)
                        frameIndex = numFrames - 2;
                    break;
                }
            }
            sampledFrames.push_back(frameIndex);
        }
    }
}
