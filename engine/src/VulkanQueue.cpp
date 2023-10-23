#include "VulkanQueue.hpp"

VkResult VulkanQueue::submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    return vkQueueSubmit2KHR(queue, submitCnt, submits, fence);
}
