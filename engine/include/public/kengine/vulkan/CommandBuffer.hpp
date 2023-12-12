#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>

struct CommandBuffer {
private:
    VkDevice vkDevice;
    const VkCommandPool pool;

public:
    const VkCommandBuffer vkCmdBuf;

    CommandBuffer(VkDevice vkDevice, VkCommandPool pool, VkCommandBuffer vkCmdBuf)
        : vkDevice(vkDevice), pool(pool), vkCmdBuf(vkCmdBuf) {}

    ~CommandBuffer() {
        vkFreeCommandBuffers(vkDevice, pool, 1, &vkCmdBuf);
    }
};