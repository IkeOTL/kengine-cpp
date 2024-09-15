#include <kengine/vulkan/SkeletonManager.hpp>
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/VmaInclude.hpp>
#include <kengine/vulkan/mesh/anim/Skeleton.hpp>
#include <tracy/Tracy.hpp>

CachedGpuBuffer& SkeletonManager::createMappedBuf(Skeleton& skeleton) {
    auto xferFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        //   | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    auto& buf = bufCache.createHostMapped(
        skeleton.size(),
        VulkanContext::FRAME_OVERLAP,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        xferFlags);

    return buf;
}

void SkeletonManager::upload(Skeleton& skeleton, int bufId, int frameIdx, float alpha) {
    ZoneScoped;

    const auto* buf = bufCache.get(bufId);

    if (!buf)
        throw std::runtime_error("No buffer found for skeleton.");

    skeleton.upload(vkCxt, *buf, frameIdx, alpha);
}