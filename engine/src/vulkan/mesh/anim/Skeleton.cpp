#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/GpuBufferCache.hpp>
#include <kengine/Math.hpp>

void Bone::setInverseBindWorldMatrix(const glm::mat4& boneOffset) {
    this->boneOffset = boneOffset;
}

void Bone::applyBindPose() {
    this->setLocalTransform(bindTransform);
}

void Bone::saveBindPose() {
    bindTransform = getLocalTransform();
}

void Skeleton::attachRootBones() {
    for (auto& b : bones)
        if (!b->getParent())
            addChild(b);
}

void Skeleton::applyBindPose() {
    for (auto& bone : bones)
        bone->applyBindPose();
}

void Skeleton::saveBindPose() {
    forceUpdateTransform();
    for (auto& bone : bones)
        bone->saveBindPose();
}

int Skeleton::getBoneCount() {
    return bones.size();
}

void Skeleton::forceUpdateBones() {
    for (int i = 0; i < bones.size(); i++)
        bones[i]->getWorldTransform();
}

void Skeleton::savePreviousTransforms() {
    auto parent = getParent();
    auto invParent = parent ? glm::inverse(parent->getWorldTransform().getTransMatrix()) : glm::mat4{};

    glm::mat4 tempMat{};
    for (int i = 0; i < prevTransforms.size(); i++) {
        auto& bone = bones[i];

        // need to undo parent transform else model is doubly transformed in shader
        if (parent)
            tempMat = invParent * bone->getWorldTransform().getTransMatrix();
        else
            tempMat = bone->getWorldTransform().getTransMatrix();

        prevTransforms[i] = tempMat;
    }
}

void Skeleton::upload(VulkanContext& vkCxt, const CachedGpuBuffer& vmaBuffer, int frameIdx, float alpha) {
    auto boneCount = bones.size();
    glm::mat4 outMats[125]; // maybe more?
    {

        auto parent = getParent();
        auto invParent = parent ? glm::inverse(parent->getWorldTransform().getTransMatrix()) : glm::mat4(1);

        glm::mat4 tempMat;
        glm::mat4 tempMatMix;
        for (int i = 0; i < boneCount; i++) {
            auto& bone = bones[i];

            if (parent)
                tempMat = invParent * bone->getWorldTransform().getTransMatrix();
            else
                tempMat = bone->getWorldTransform().getTransMatrix();

            glm::vec3 cPos = glm::vec3(tempMat[3]);
            // NOTE: glm::quat_cast/toQuat doesnt include scale! if there are issues with animations investigate (orig: JOML mat4.getUnnormalizedRotation)
            //glm::quat cRot = glm::quat_cast(tempMat);
            glm::quat cRot;
            math::getUnnormalizedRotation(tempMat, cRot);
            glm::vec3 cScl = glm::vec3(glm::length(tempMat[0]), glm::length(tempMat[1]), glm::length(tempMat[2]));

            glm::vec3 prevPos = glm::vec3(prevTransforms[i][3]);
            // NOTE: glm::quat_cast/toQuat doesnt include scale! if there are issues with animations investigate (orig: JOML mat4.getUnnormalizedRotation)
            //glm::quat prevRot = glm::quat_cast(prevTransforms[i]);
            glm::quat prevRot;
            math::getUnnormalizedRotation(prevTransforms[i], prevRot);
            glm::vec3 prevScl = glm::vec3(glm::length(prevTransforms[i][0]), glm::length(prevTransforms[i][1]), glm::length(prevTransforms[i][2]));

            glm::vec3 mPos = glm::mix(prevPos, cPos, alpha);
            glm::quat mRot = glm::lerp(prevRot, cRot, alpha); // maybe slerp?
            glm::vec3 mScl = glm::mix(prevScl, cScl, alpha);

            tempMatMix = glm::translate(glm::mat4(1.0f), mPos) * glm::mat4_cast(mRot) * glm::scale(glm::mat4(1.0f), mScl);

            // write to buf or to mem chunk for a single write?
            outMats[i] = tempMatMix * bone->getInverseBindWorldMatrix();
        }
    }

    auto size = boneCount * sizeof(glm::mat4);
    auto pos = vmaBuffer.getFrameOffset(frameIdx);

    auto buf = static_cast<unsigned char*>(vmaBuffer.getGpuBuffer().data());
    memcpy(buf + pos, outMats, size);

    vmaBuffer.getGpuBuffer().flush(pos, size);
}