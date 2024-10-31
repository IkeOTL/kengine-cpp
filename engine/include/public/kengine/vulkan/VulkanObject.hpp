#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <kengine/vulkan/VmaInclude.hpp>

namespace ke {
    template <class T>
    class HandleWrapper {
    public:
        const T handle;

        HandleWrapper(T handle)
            : handle(handle) {}

        virtual ~HandleWrapper() = default;
    };

    class VulkanInstance : public HandleWrapper<VkInstance> {
    public:
        VulkanInstance(VkInstance handle)
            : HandleWrapper(handle) {}

        ~VulkanInstance() {
            vkDestroyInstance(handle, nullptr);
        }
    };

    class VulkanSurface : public HandleWrapper<VkSurfaceKHR> {
    private:
        const VkInstance vkInstance;

    public:
        VulkanSurface(VkInstance vkInstance, VkSurfaceKHR handle)
            : HandleWrapper(handle),
              vkInstance(vkInstance) {}

        ~VulkanSurface() {
            vkDestroySurfaceKHR(vkInstance, handle, nullptr);
        }
    };

    class VulkanDevice : public HandleWrapper<VkDevice> {
    public:
        VulkanDevice(VkDevice handle)
            : HandleWrapper(handle) {}

        ~VulkanDevice() {
            vkDestroyDevice(handle, nullptr);
        }
    };

    class VulkanAllocator : public HandleWrapper<VmaAllocator> {
    public:
        VulkanAllocator(VmaAllocator handle)
            : HandleWrapper(handle) {}

        ~VulkanAllocator() {
            vmaDestroyAllocator(handle);
        }
    };

    class VulkanSemaphore : public HandleWrapper<VkSemaphore> {
    private:
        const VkDevice vkDevice;

    public:
        VulkanSemaphore(VkDevice vkDevice, VkSemaphore handle)
            : HandleWrapper(handle),
              vkDevice(vkDevice) {}

        inline static std::unique_ptr<VulkanSemaphore> create(VkDevice vkDevice, VkSemaphore handle) {
            return std::make_unique<VulkanSemaphore>(vkDevice, handle);
        }

        ~VulkanSemaphore() {
            vkDestroySemaphore(vkDevice, handle, nullptr);
        }
    };

    class VulkanFence : public HandleWrapper<VkFence> {
    private:
        const VkDevice vkDevice;

    public:
        VulkanFence(VkDevice vkDevice, VkFence handle)
            : HandleWrapper(handle),
              vkDevice(vkDevice) {}

        inline static std::unique_ptr<VulkanFence> create(VkDevice vkDevice, VkFence handle) {
            return std::make_unique<VulkanFence>(vkDevice, handle);
        }

        ~VulkanFence() {
            vkDestroyFence(vkDevice, handle, nullptr);
        }
    };

    class VulkanCommandPool : public HandleWrapper<VkCommandPool> {
    private:
        const VkDevice vkDevice;

    public:
        VulkanCommandPool(VkDevice vkDevice, VkCommandPool handle)
            : HandleWrapper(handle),
              vkDevice(vkDevice) {}

        ~VulkanCommandPool() {
            vkDestroyCommandPool(vkDevice, handle, nullptr);
        }
    };

    class VulkanSampler : public HandleWrapper<VkSampler> {
    private:
        const VkDevice vkDevice;

    public:
        VulkanSampler(VkDevice vkDevice, VkSampler handle)
            : HandleWrapper(handle),
              vkDevice(vkDevice) {}

        ~VulkanSampler() {
            vkDestroySampler(vkDevice, handle, nullptr);
        }
    };

    class VulkanShaderModule : public HandleWrapper<VkShaderModule> {
    private:
        const VkDevice vkDevice;

    public:
        VulkanShaderModule(VkDevice vkDevice, VkShaderModule handle)
            : HandleWrapper(handle),
              vkDevice(vkDevice) {}

        ~VulkanShaderModule() {
            vkDestroyShaderModule(vkDevice, handle, nullptr);
        }
    };

    class VulkanDescriptorPool : public HandleWrapper<VkDescriptorPool> {
    private:
        const VkDevice vkDevice;

    public:
        VulkanDescriptorPool(VkDevice vkDevice, VkDescriptorPool handle)
            : HandleWrapper(handle),
              vkDevice(vkDevice) {}

        ~VulkanDescriptorPool() {
            vkDestroyDescriptorPool(vkDevice, handle, nullptr);
        }
    };
} // namespace ke