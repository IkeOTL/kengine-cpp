#include <kengine/vulkan/ShadowContext.hpp>

void ShadowContext::init(VulkanContext& vkContext, std::vector<DescriptorSetAllocator>& descSetAllocators,
	glm::vec3 lightDir, CachedGpuBuffer& drawObjectBuf, CachedGpuBuffer drawInstanceBuffer) {
	auto xferFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		| VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
		| VMA_ALLOCATION_CREATE_MAPPED_BIT;

	shadowPassCascadeBuf = &bufCache.createHostMapped(
		ShadowCascadeData::SHADOW_CASCADE_COUNT * 16 * sizeof(float),
		VulkanContext::FRAME_OVERLAP,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO,
		xferFlags);

	compositePassCascadeBuf = &bufCache.createHostMapped(
		ShadowCascadeData::size(),
		VulkanContext::FRAME_OVERLAP,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO,
		xferFlags);
}

void ShadowContext::execute(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt, DescriptorSetAllocator& dAllocator,
	IndirectDrawBatch* nonSkinnedBatches, size_t nonSkinnedBatchesSize,
	IndirectDrawBatch* skinnedBatches, size_t skinnedBatchesSize) {
	const auto SHADOWDIM = 4096;

	auto camera = cameraController.getCamera();

	auto nearClip = camera->getNearClip();
	auto farClip = camera->getFarClip();
	auto clipRange = farClip - nearClip;

	auto minZ = nearClip;
	auto maxZ = nearClip + clipRange;

	auto range = maxZ - minZ;
	auto ratio = maxZ / minZ;

	glm::mat4 invCam{};
	camera->getViewMatrix(invCam);
	invCam = glm::inverse(camera->getProjectionMatrix() * invCam);

	auto cascadeSplitLambda = 0.75f;
	auto lastDistSplit = 0.0f;
	for (int i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {

	}

	cascadesData.uploadShadowPass(vkContext, *shadowPassCascadeBuf, cxt.frameIndex);
	cascadesData.uploadCompositionPass(vkContext, *compositePassCascadeBuf, cxt.frameIndex);

	for (int i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {

	}
}

void ShadowContext::execShadowPass(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt,
	Pipeline& p1, DescriptorSetAllocator& dAllocator, size_t cascadeIdx,
	IndirectDrawBatch* batches, size_t batchesSize, bool skinned) {
}
