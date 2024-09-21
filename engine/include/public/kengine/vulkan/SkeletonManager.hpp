#pragma once
#include<memory>


class VulkanContext;
class GpuBufferCache;
class CachedGpuBuffer;
class Skeleton;

class SkeletonManager {
private:
    VulkanContext& vkContext;

public:
    SkeletonManager(VulkanContext& vkCxt)
        : vkContext(vkCxt) {}

    inline static std::unique_ptr<SkeletonManager> create(VulkanContext& vkCxt) {
        return std::make_unique<SkeletonManager>(vkCxt);
    }

    CachedGpuBuffer& createMappedBuf(Skeleton& skeleton);
    void upload(Skeleton& skeleton, int bufId, int frameIdx, float alpha);
};