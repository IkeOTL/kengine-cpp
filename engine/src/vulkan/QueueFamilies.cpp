#include "QueueFamilies.hpp"

void QueueFamilies::init(VkPhysicalDevice physicalDevice) {
    auto queueFamilyCount = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties2> allQueueFamilies(queueFamilyCount);
    for (auto& qf : allQueueFamilies) {
        qf.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        qf.pNext = nullptr;
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, allQueueFamilies.data());

    for (auto i = 0; i < queueFamilyCount; i++) {
        const auto& queueFamily = allQueueFamilies[i].queueFamilyProperties;
        const auto flags = queueFamily.queueFlags;

        if ((flags & VK_QUEUE_GRAPHICS_BIT) &&
            (flags & VK_QUEUE_COMPUTE_BIT) &&
            (flags & VK_QUEUE_TRANSFER_BIT)) {
            gfxCompXfer.push_back(i);
            continue;
        }

        if ((flags & VK_QUEUE_TRANSFER_BIT) &&
            !(flags & VK_QUEUE_GRAPHICS_BIT))
            transfer.push_back(i);
    }
}
