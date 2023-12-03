#pragma once
#include <VulkanInclude.hpp>
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
        return transfer;
    }
};