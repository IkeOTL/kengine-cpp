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
}

VkResult VulkanQueue::submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence) {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    return pfnVkQueueSubmit2KHR(queue, submitCnt, submits, fence);
    //return vkQueueSubmit2KHR(queue, submitCnt, submits, fence);
}
