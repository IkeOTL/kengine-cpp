#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/CommandBuffer.hpp>

#include <mutex>
#include <vector>

class VulkanContext;

class CommandPool {
private:
    VkDevice vkDevice;

    static thread_local VkCommandPool gfxPool;
    static thread_local VkCommandPool xferPool;
    static thread_local VkCommandPool computePool;

    std::vector<CommandBuffer> framePool;

    void initThread(VulkanContext& vkContext);

    std::unique_ptr<CommandBuffer> createCommandBuffer(VkCommandPool, VkCommandBufferLevel level);
    std::vector<std::unique_ptr<CommandBuffer>> createCommandBuffers(VkCommandPool, VkCommandBufferLevel level, uint32_t count);
    VkCommandPool createCommandPool(VkCommandPoolCreateFlags flags, uint32_t fam);

public:
    CommandPool(VkDevice vkDevice)
        : vkDevice(vkDevice) {}

    std::unique_ptr<CommandBuffer> createGraphicsCmdBuf();
    std::unique_ptr<CommandBuffer> createComputeCmdBuf();
    std::unique_ptr<CommandBuffer> createTransferCmdBuf();
};