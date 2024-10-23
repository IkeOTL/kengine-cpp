#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/CommandBuffer.hpp>
#include <kengine/vulkan/VulkanObject.hpp>

#include <functional>
#include <mutex>
#include <vector>

class VulkanContext;

class CommandPool {
private:
    VkDevice vkDevice;

    static thread_local std::unique_ptr<ke::VulkanCommandPool> gfxPool;
    static thread_local std::unique_ptr<ke::VulkanCommandPool> xferPool;
    static thread_local std::unique_ptr<ke::VulkanCommandPool> computePool;

    std::unique_ptr<CommandBuffer> createCommandBuffer(VkCommandPool, VkCommandBufferLevel level);
    std::vector<std::unique_ptr<CommandBuffer>> createCommandBuffers(VkCommandPool, VkCommandBufferLevel level, uint32_t count);
    VkCommandPool createCommandPool(VkCommandPoolCreateFlags flags, uint32_t fam);

public:
    CommandPool(VkDevice vkDevice)
        : vkDevice(vkDevice) {}

    ~CommandPool();

    void initThread(VulkanContext& vkContext);

    std::unique_ptr<CommandBuffer> createGraphicsCmdBuf();
    std::unique_ptr<CommandBuffer> createComputeCmdBuf();
    std::unique_ptr<CommandBuffer> createTransferCmdBuf();
};