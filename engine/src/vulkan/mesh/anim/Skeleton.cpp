#include <kengine/vulkan/mesh/anim/Skeleton.hpp>

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

void Skeleton::upload(VulkanContext& vkCxt, CachedGpuBuffer& vmaBuffer, int frameIdx, float alpha) {
}