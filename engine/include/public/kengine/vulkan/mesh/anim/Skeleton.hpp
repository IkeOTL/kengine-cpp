#pragma once
#include <kengine/Spatial.hpp>
#include <glm/mat4x4.hpp>

class VulkanContext;
class CachedGpuBuffer;

class Bone : public Spatial {
private:
    Transform bindTransform{};
    glm::mat4 boneOffset{};

    const int boneId;
public:
    Bone(int boneId, std::string name)
        : Spatial(name), boneId(boneId) {}

    void applyBindPose();
    void saveBindPose();
};

class Skeleton : public Spatial {
private:
    std::vector<glm::mat4> prevTransforms;
    std::vector<std::shared_ptr<Bone>> bones;

    void attachRootBones();

public:
    Skeleton(std::string name, std::vector<std::shared_ptr<Bone>> bones)
        : Spatial(name), bones(bones) {
        attachRootBones();
        prevTransforms.resize(this->bones.size());
    };

    const std::vector<std::shared_ptr<Bone>>& getBones() const {
        return bones;
    }

    int size() {
        return singleSize() * bones.size();
    }

    int singleSize() {
        return sizeof(glm::mat4);
    }

    void applyBindPose();
    void saveBindPose();
    int getBoneCount();
    void forceUpdateBones();
    void savePreviousTransforms();
    void upload(VulkanContext& vkCxt, CachedGpuBuffer& vmaBuffer, int frameIdx, float alpha);
};