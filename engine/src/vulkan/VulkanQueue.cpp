#include <kengine/vulkan/VulkanQueue.hpp>

namespace ke {
    void VulkanQueue::init() {
        vkGetDeviceQueue(vkDevice, famIdx, 0, &queue);
    }

    VkResult VulkanQueue::submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence) {
        std::lock_guard<std::mutex> lock(mtx);
        return vkQueueSubmit2(queue, submitCnt, submits, fence);
        //return vkQueueSubmit2KHR(queue, submitCnt, submits, fence);
    }

    VkResult VulkanQueue::present(VkPresentInfoKHR* presentInfo) {
        std::lock_guard<std::mutex> lock(mtx);
        return vkQueuePresentKHR(queue, presentInfo);
    }

    VkResult VulkanQueue::waitIdle() {
        std::lock_guard<std::mutex> lock(mtx);
        return vkQueueWaitIdle(queue);
    }
} // namespace ke