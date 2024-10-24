#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <mutex>

namespace ke {
    class VulkanQueue {

    private:
        const VkDevice vkDevice;
        const unsigned int famIdx;
        VkQueue queue = VK_NULL_HANDLE;

        // sync queue commands
        std::mutex mtx{};

    public:
        VulkanQueue(VkDevice vkDevice, unsigned int famIdx)
            : vkDevice(vkDevice), famIdx(famIdx) {}

        ~VulkanQueue() {}

        void init();

        VkResult submit(unsigned int submitCnt, VkSubmitInfo2* submits, VkFence fence);
        VkResult present(VkPresentInfoKHR* presentInfo);
        VkResult waitIdle();

        VkQueue getVkQueue() const {
            return queue;
        }
    };
} // namespace ke