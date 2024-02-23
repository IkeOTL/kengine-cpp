#include <kengine/vulkan/mesh/anim/Skeleton.hpp>


void Bone::applyBindPose() {
    this->setLocalTransform(bindTransform);
}

void Skeleton::attachRootBones() {
    for (auto& b : bones)
        if (!b->getParent())
            addChild(b);
}

void Skeleton::applyBindPose() {
    for (auto& b : bones)
        b->applyBindPose();
}

