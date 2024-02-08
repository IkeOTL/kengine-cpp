#pragma once
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/MaterialBindManager.hpp>
#include <kengine/vulkan/mesh/Mesh.hpp>
#include <kengine/vulkan/material/Material.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

class IndirectDrawBatch {

private:
    Mesh* mesh;
    Material* material;

    uint32_t cmdId = 0;
    uint32_t firstInstanceIdx = 0;
    uint32_t instanceCount = 0;

public:
    void predraw(VulkanContext& vkCxt, VkCommandBuffer vkCmd, MaterialBindManager& bindManager,
        DescriptorSetAllocator& descSetAllocator, int frameIdx);

    void draw(VulkanContext& vkCxt, VkCommandBuffer vkCmd, long indirectCmdBuf, DescriptorSetAllocator& descSetAllocator,
        int frameIdx, MaterialBindManager& bindManager);
    
    uint32_t getCmdId() {
        return cmdId;
    }

    Mesh* getMesh() {
        return mesh;
    }

    Material* getMaterial() {
        return material;
    }
};