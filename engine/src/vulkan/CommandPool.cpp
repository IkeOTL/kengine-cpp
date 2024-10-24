#include <kengine/vulkan/CommandPool.hpp>
#include <kengine/vulkan/VulkanContext.hpp>

namespace ke {
    thread_local std::unique_ptr<ke::VulkanCommandPool> CommandPool::gfxPool = VK_NULL_HANDLE;
    thread_local std::unique_ptr<ke::VulkanCommandPool> CommandPool::xferPool = VK_NULL_HANDLE;
    thread_local std::unique_ptr<ke::VulkanCommandPool> CommandPool::computePool = VK_NULL_HANDLE;

    CommandPool::~CommandPool() {
        gfxPool.reset();
        xferPool.reset();
        computePool.reset();
    }

    void CommandPool::initThread(VulkanContext& vkContext) {
        auto gfx = createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, vkContext.getGfxQueueFamilyIndex());
        gfxPool = std::make_unique<ke::VulkanCommandPool>(vkDevice, gfx);

        auto compute = createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, vkContext.getGfxQueueFamilyIndex());
        computePool = std::make_unique<ke::VulkanCommandPool>(vkDevice, compute);

        // used for transfer operations, like staging data
        auto xfer = createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, vkContext.getXferQueueFamilyIndex());
        xferPool = std::make_unique<ke::VulkanCommandPool>(vkDevice, xfer);
    }

    std::unique_ptr<CommandBuffer> CommandPool::createGraphicsCmdBuf() {
        return createCommandBuffer(gfxPool->handle, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    std::unique_ptr<CommandBuffer> CommandPool::createComputeCmdBuf() {
        return createCommandBuffer(computePool->handle, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    std::unique_ptr<CommandBuffer> CommandPool::createTransferCmdBuf() {
        return createCommandBuffer(xferPool->handle, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    VkCommandPool CommandPool::createCommandPool(VkCommandPoolCreateFlags flags, uint32_t fam) {
        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = fam;
        poolCreateInfo.flags = flags;

        VkCommandPool cmdPool;
        VKCHECK(vkCreateCommandPool(vkDevice, &poolCreateInfo, nullptr, &cmdPool),
            "Failed to create command pool");

        return cmdPool;
    }

    std::unique_ptr<CommandBuffer> CommandPool::createCommandBuffer(VkCommandPool cmdPool, VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = cmdPool;
        cmdBufAllocateInfo.level;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer cmdBuf;
        VKCHECK(vkAllocateCommandBuffers(vkDevice, &cmdBufAllocateInfo, &cmdBuf),
            "Failed to allocate command buffer");

        return std::make_unique<CommandBuffer>(vkDevice, cmdPool, cmdBuf);
    }

    std::vector<std::unique_ptr<CommandBuffer>> CommandPool::createCommandBuffers(VkCommandPool cmdPool,
        VkCommandBufferLevel level, uint32_t count) {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = cmdPool;
        cmdBufAllocateInfo.level;
        cmdBufAllocateInfo.commandBufferCount = count;

        std::vector<VkCommandBuffer> cmdBufs(count);
        VKCHECK(vkAllocateCommandBuffers(vkDevice, &cmdBufAllocateInfo, cmdBufs.data()),
            "Failed to allocate command buffers");

        std::vector<std::unique_ptr<CommandBuffer>> ret;
        for (auto i = 0; i < count; i++)
            ret.push_back(std::make_unique<CommandBuffer>(vkDevice, cmdPool, cmdBufs[i]));

        return ret;
    }
} // namespace ke