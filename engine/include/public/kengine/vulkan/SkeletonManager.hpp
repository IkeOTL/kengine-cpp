#pragma once

class VulkanContext;
class GpuBufferCache;
class CachedGpuBuffer;
class Skeleton;

class SkeletonManager {
private:
    VulkanContext& vkCxt;
    GpuBufferCache& bufCache;

public:
    SkeletonManager(VulkanContext& vkCxt, GpuBufferCache& bufCache)
        : vkCxt(vkCxt), bufCache(bufCache) {}

    CachedGpuBuffer& createMappedBuf(Skeleton& skeleton);
    void upload(Skeleton& skeleton, int bufId, int frameIdx, float alpha);
};