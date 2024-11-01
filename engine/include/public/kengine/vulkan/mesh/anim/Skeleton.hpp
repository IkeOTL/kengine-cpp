#pragma once
#include <kengine/Spatial.hpp>
#include <glm/mat4x4.hpp>

namespace ke {
    class VulkanContext;
    class CachedGpuBuffer;

    class Bone : public Spatial {
    private:
        Transform bindTransform{};
        glm::mat4 boneOffset{1};

        const uint32_t boneId;

    public:
        Bone(uint32_t boneId, std::string name)
            : Spatial(name),
              boneId(boneId) {}

        void setInverseBindWorldMatrix(const glm::mat4& boneOffset);

        const glm::mat4& getInverseBindWorldMatrix() const {
            return boneOffset;
        }

        const uint32_t getBoneId() const {
            return boneId;
        }

        const Transform& getBindPose() const {
            return bindTransform;
        }

        void applyBindPose();
        void saveBindPose();
    };

    class Skeleton : public Spatial {
    private:
        std::vector<glm::mat4> prevTransforms;
        std::vector<std::shared_ptr<Bone>> bones;

        void attachRootBones();

    public:
        Skeleton(std::string name, std::vector<std::shared_ptr<Bone>>&& bones)
            : Spatial(name),
              bones(std::move(bones)) {
            attachRootBones();
            prevTransforms.resize(this->bones.size());
        };

        const std::vector<std::shared_ptr<Bone>>& getBones() const {
            return bones;
        }

        void applyBindPose();
        void saveBindPose();
        int getBoneCount();
        void forceUpdateBones();
        void savePreviousTransforms();
        void upload(VulkanContext& vkCxt, const CachedGpuBuffer& vmaBuffer, int frameIdx, float alpha);

        uint32_t size() {
            return singleSize() * bones.size();
        }

        static uint32_t singleSize() {
            return sizeof(glm::mat4);
        }
    };
} // namespace ke