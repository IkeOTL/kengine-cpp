#pragma once
#include <vulkan/vulkan.h>
#include <mutex>

class VulkanQueue {

private:
    const VkDevice vkDevice;
    const unsigned int famIdx;
    VkQueue queue = VK_NULL_HANDLE;

    PFN_vkQueueSubmit2KHR pfnVkQueueSubmit2KHR = nullptr;

    // sync queue commands
    std::recursive_mutex mtx;

public:
    VulkanQueue(VkDevice vkDevice, unsigned int famIdx);
    ~VulkanQueue();

    void init();

    VkResult submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence);
};