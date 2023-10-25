#include "VulkanQueue.hpp"

VulkanQueue::VulkanQueue(VkDevice vkDevice, unsigned int famIdx)
    :vkDevice(vkDevice), famIdx(famIdx) {}

VulkanQueue::~VulkanQueue() {

}

void VulkanQueue::init() {
    vkGetDeviceQueue(vkDevice, famIdx, 0, &queue);

    // load extension functions
    pfnVkQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)vkGetDeviceProcAddr(vkDevice, "vkQueueSubmit2KHR");
    if (!pfnVkQueueSubmit2KHR)
        throw std::runtime_error("Failed to load PFN_vkQueueSubmit2KHR.");

    pfnVkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(vkDevice, "vkQueuePresentKHR");
    if (!pfnVkQueuePresentKHR)
        throw std::runtime_error("Failed to load PFN_vkQueuePresentKHR.");
}

VkResult VulkanQueue::submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence) {
    std::lock_guard<std::mutex> lock(mtx);
    return pfnVkQueueSubmit2KHR(queue, submitCnt, submits, fence);
    //return vkQueueSubmit2KHR(queue, submitCnt, submits, fence);
}

VkResult VulkanQueue::present(VkPresentInfoKHR* presentInfo) {
    std::lock_guard<std::mutex> lock(mtx);
    return pfnVkQueuePresentKHR(queue, presentInfo);
}

VkResult VulkanQueue::waitIdle() {
    std::lock_guard<std::mutex> lock(mtx);
    return vkQueueWaitIdle(queue);
}