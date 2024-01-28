#include <kengine/vulkan/ShadowContext.hpp>
#include <kengine/vulkan/pipelines/CascadeShadowMapPipeline.hpp>

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

	glm::mat4 invCamViewProj{};
	camera->getViewMatrix(invCamViewProj);
	invCamViewProj = glm::inverse(camera->getProjectionMatrix() * invCamViewProj);

	auto cascadeSplitLambda = 0.75f;
	auto lastDistSplit = 0.0f;
	for (int i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {
		auto p = (i + 1.0f) / (float)ShadowCascadeData::SHADOW_CASCADE_COUNT;
		auto log = minZ * std::powf(ratio, p);
		auto uniform = minZ + range * p;
		auto d = cascadeSplitLambda * (log - uniform) + uniform;
		auto splitDist = (d - nearClip) / clipRange;

		auto cascade = cascadesData.getCascade(i);
		cascade->updateViewProj(invCamViewProj, camera->getNearClip(), sceneData.getLightDir(),
			lastDistSplit, splitDist, clipRange);

		lastDistSplit = splitDist;
	}

	cascadesData.uploadShadowPass(vkContext, *shadowPassCascadeBuf, cxt.frameIndex);
	cascadesData.uploadCompositionPass(vkContext, *compositePassCascadeBuf, cxt.frameIndex);

	for (int i = 0; i < ShadowCascadeData::SHADOW_CASCADE_COUNT; i++) {
		auto rp1Cxt = RenderPass::RenderPassContext{ 1, i, cxt.cmd, glm::uvec2(SHADOWDIM) };
		vkContext.beginRenderPass(rp1Cxt);
		{
			auto pipeline = vkContext.getPipelineCache().getPipeline<CascadeShadowMapPipeline>();
			execShadowPass(vkContext, cxt, *pipeline, dAllocator, i, nonSkinnedBatches, nonSkinnedBatchesSize, false);

			auto skinnedPipeline = vkContext.getPipelineCache().getPipeline<SkinnedCascadeShadowMapPipeline>();
			execShadowPass(vkContext, cxt, skinnedPipeline, dAllocator, i, skinnedBatches, skinnedBatchesSize, true);
		}
		vkContext.endRenderPass(rp1Cxt);
	}
}

void ShadowContext::execShadowPass(VulkanContext& vkContext, VulkanContext::RenderFrameContext& cxt,
	Pipeline& p1, DescriptorSetAllocator& dAllocator, size_t cascadeIdx,
	IndirectDrawBatch* batches, size_t batchesSize, bool skinned) {
}
