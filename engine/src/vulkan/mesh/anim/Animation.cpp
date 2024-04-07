#include <kengine/vulkan/mesh/anim/Animation.hpp>
#include <kengine/Transform.hpp>

Animation::Animation(std::string name, float ticksPerSecond, float duration, std::vector<BoneTrack>&& tracks)
    : name(name), ticksPerSecond(ticksPerSecond), duration(duration), tracks(std::move(tracks)) {
    calculateMaxRange();
    updateSamples();
}

float Animation::apply(Skeleton& skeleton, float time, bool loop) {
    auto clamp = [](int input, int min, int max) { return (input < min) ? min : (input > max) ? max : input; };

    time = adjustTime(time, loop);

    auto& bones = skeleton.getBones();
    for (auto& bone : bones) {
        if (bone->getBoneId() >= tracks.size())
            continue;

        auto& track = tracks[bone->getBoneId()];
        auto& times = track.getTimes();
        int frameCount = track.getFrameCount();
        int finalFrameIdx = frameCount - 1;

        int startFrameIdx = getTargetFrameIdx(bone->getBoneId(), time, loop);
        int nextFrameIdx = clamp(startFrameIdx + 1, 0, finalFrameIdx);
        float blend = 0.0f;

        if (startFrameIdx == finalFrameIdx) {
            float guessedInterval = times[finalFrameIdx] - times[finalFrameIdx - 1];

            if (guessedInterval > 0.0f)
                blend = (time - times[startFrameIdx]) / guessedInterval;
        }
        else
            blend = (time - times[startFrameIdx]) / (times[startFrameIdx + 1] - times[startFrameIdx]);


        // NOTE: for some reason blend is coming out to under 0 and over 1
        // need to fix
        /*if (startFrameIdx == finalFrameIdx || blend < 0) {
            track.getTranslation(startFrameIdx).get(bone.getLocalTransform().getPosition());
            track.getRotation(startFrameIdx).get(bone.getLocalTransform().getRotation());
            track.getScale(startFrameIdx).get(bone.getLocalTransform().getScale());
            continue;
        }

        if (blend > 1) {
            track.getTranslation(nextFrameIdx).get(bone.getLocalTransform().getPosition());
            track.getRotation(nextFrameIdx).get(bone.getLocalTransform().getRotation());
            track.getScale(nextFrameIdx).get(bone.getLocalTransform().getScale());
            continue;
        }*/

        auto iPos = glm::mix(track.getTranslation(startFrameIdx), track.getTranslation(nextFrameIdx), blend);
        auto iRot = glm::lerp(track.getRotation(startFrameIdx), track.getRotation(nextFrameIdx), blend);
        auto iScl = glm::mix(track.getScale(startFrameIdx), track.getScale(nextFrameIdx), blend);
        bone->getLocalTransform().set(iPos, iRot, iScl);
    }

    skeleton.forceUpdateTransform();

    return time;
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
