#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VmaInclude.hpp>

namespace ke {
    class VulkanInstance {
    private:
        const VkInstance vkInstance;

    public:
        VulkanInstance(VkInstance vkInstance)
            : vkInstance(vkInstance) {}

        inline static std::unique_ptr<VulkanInstance> create(VkInstance vkInstance) {
            return std::make_unique<VulkanInstance>(vkInstance);
        }

        VkInstance getVkInstance() const {
            return vkInstance;
        }

        ~VulkanInstance() {
            vkDestroyInstance(vkInstance, nullptr);
        }
    };

    class VulkanAllocator {
    private:
        const VmaAllocator vmaAllocator;

    public:
        VulkanAllocator(VmaAllocator vmaAllocator)
            : vmaAllocator(vmaAllocator) {}

        inline static std::unique_ptr<VulkanAllocator> create(VmaAllocator vmaAllocator) {
            return std::make_unique<VulkanAllocator>(vmaAllocator);
        }

        VmaAllocator getVmaAllocator() const {
            return vmaAllocator;
        }

        ~VulkanAllocator() {
            vmaDestroyAllocator(vmaAllocator);
        }
    };

    class VulkanSemaphore {
    private:
        const VkDevice vkDevice;
        const VkSemaphore vkSemaphore;

    public:
        VulkanSemaphore(VkDevice vkDevice, VkSemaphore vkSemaphore)
            : vkDevice(vkDevice), vkSemaphore(vkSemaphore) {}

        inline static std::unique_ptr<VulkanSemaphore> create(VkDevice vkDevice, VkSemaphore vkSemaphore) {
            return std::make_unique<VulkanSemaphore>(vkDevice, vkSemaphore);
        }

        VkSemaphore getVkSemaphore() const {
            return vkSemaphore;
        }

        ~VulkanSemaphore() {
            vkDestroySemaphore(vkDevice, vkSemaphore, nullptr);
        }
    };

    class VulkanFence {
    private:
        const VkDevice vkDevice;
        const VkFence vkFence;

    public:
        VulkanFence(VkDevice vkDevice, VkFence vkFence)
            : vkDevice(vkDevice), vkFence(vkFence) {}

        inline static std::unique_ptr<VulkanFence> create(VkDevice vkDevice, VkFence vkFence) {
            return std::make_unique<VulkanFence>(vkDevice, vkFence);
        }

        VkFence getVkFence() const {
            return vkFence;
        }

        ~VulkanFence() {
            vkDestroyFence(vkDevice, vkFence, nullptr);
        }
    };
}