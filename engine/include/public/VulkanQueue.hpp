#pragma once
#include <vulkan/vulkan.h>
#include <mutex>

class VulkanQueue {

private:
    const VkDevice vkDevice;
    const unsigned int famIdx;
    VkQueue queue = VK_NULL_HANDLE;

    PFN_vkQueueSubmit2KHR pfnVkQueueSubmit2KHR = nullptr;
    PFN_vkQueuePresentKHR pfnVkQueuePresentKHR = nullptr;

    // sync queue commands
    std::mutex mtx;

public:
    VulkanQueue(VkDevice vkDevice, unsigned int famIdx)
        : vkDevice(vkDevice), famIdx(famIdx) {}

    ~VulkanQueue() {}

    void init();

    VkResult submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence);
    VkResult present(VkPresentInfoKHR* presentInfo);
    VkResult waitIdle();
};