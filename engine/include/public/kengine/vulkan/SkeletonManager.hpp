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
    CachedGpuBuffer& createMappedBuf(Skeleton& skeleton);
    void upload(Skeleton& skeleton, int bufId, int frameIdx, float alpha);
};