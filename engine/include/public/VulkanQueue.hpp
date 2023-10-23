#pragma once
#include <vulkan/vulkan.h>
#include <mutex>

class VulkanQueue {

private:
    const VkDevice device;
    const unsigned int famIdx;
    VkQueue queue;

    // sync queue commands
    std::recursive_mutex mtx;

public:
    VulkanQueue(VkDevice device, unsigned int famIdx);
    ~VulkanQueue();

    void init();

    VkResult submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence);
};