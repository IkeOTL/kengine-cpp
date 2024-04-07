#include <kengine/vulkan/mesh/anim/BoneTrack.hpp>

void LinearBoneTrack::setFrame(Transform& bTransform, uint32_t frameIdx) const {
}

void LinearBoneTrack::mixFrames(Transform& bTransform, uint32_t startFrameIdx, uint32_t nextFrameIdx, float factor) const {
}

void CubicSplineBoneTrack::setFrame(Transform& bTransform, uint32_t frameIdx) const {
}

void CubicSplineBoneTrack::mixFrames(Transform& bTransform, uint32_t startFrameIdx, uint32_t nextFrameIdx, float factor) const {
}