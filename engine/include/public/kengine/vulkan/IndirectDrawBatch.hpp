#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

class VulkanContext;
class DescriptorSetAllocator;
class MaterialBindManager;
class Mesh;
class Material;

class IndirectDrawBatch {

private:
    const Mesh* mesh;
    const Material* material;

    uint32_t cmdId = 0;
    uint32_t firstInstanceIdx = 0;
    uint32_t instanceCount = 0;

public:
    void predraw(VulkanContext& vkCxt, VkCommandBuffer vkCmd, MaterialBindManager& bindManager,
        DescriptorSetAllocator& descSetAllocator, int frameIdx);

    void draw(VulkanContext& vkCxt, VkCommandBuffer vkCmd, VkBuffer indirectCmdBuf, DescriptorSetAllocator& descSetAllocator,
        int frameIdx, MaterialBindManager& bindManager);

    void reset() {
        mesh = nullptr;
        material = nullptr;
        cmdId = 0;
        firstInstanceIdx = 0;
        instanceCount = 0;
    }

    uint32_t getCmdId() {
        return cmdId;
    }

    void setCmdId(uint32_t id) {
        cmdId = id;
    }

    const Mesh* getMesh() {
        return mesh;
    }

    void setMesh(const Mesh* m) {
        mesh = m;
    }

    const Material* getMaterial() {
        return material;
    }

    void setMaterial(const Material* m) {
        material = m;
    }

    uint32_t getFirstInstanceIdx() {
        return firstInstanceIdx;
    }

    void setFirstInstanceIdx(uint32_t i) {
        firstInstanceIdx = i;
    }

    uint32_t getInstanceCount() {
        return instanceCount;
    }

    void setInstanceCount(uint32_t i) {
        instanceCount = i;
    }

    void incrementInstanceCount() {
        instanceCount++;
    }
};