#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <stdexcept>
#include <vector>

class QueueFamilies {
private:
    std::vector<unsigned int> gfxCompXfer;
    std::vector<unsigned int> transfer;

public:
    void init(VkPhysicalDevice physicalDevice);

    std::vector<unsigned int>& getGfxCompXferQueues() {
        return gfxCompXfer;
    }

    std::vector<unsigned int>& getTransferQueues() {
        // if dedicated transfer queue not find, use graphics queue
        return transfer.size() ? transfer : gfxCompXfer;
    }
};