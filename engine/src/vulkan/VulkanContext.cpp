#define VMA_IMPLEMENTATION
#include <kengine/vulkan/VulkanContext.hpp>
#include <kengine/vulkan/renderpass/RenderPass.hpp>
#include <kengine/vulkan/GpuBuffer.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

VulkanContext::VulkanContext(RenderPassCreator&& renderPassCreator, SwapchainCreator::OnSwapchainCreate&& onSwapchainCreate)
	: renderPassCreator(std::move(renderPassCreator)),
	swapchainCreator(std::move(onSwapchainCreate)) {}

VulkanContext::~VulkanContext() {
	if (vkInstance != VK_NULL_HANDLE) {
		auto funcDestroyDebug = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
		if (funcDestroyDebug)
			funcDestroyDebug(vkInstance, debugCallbackHandle, nullptr);

		vmaDestroyAllocator(vmaAllocator);

		vkDestroyInstance(vkInstance, nullptr);
	}
}

void VulkanContext::init(Window& window, bool validationOn) {
	createVkInstance(validationOn);

	if (validationOn)
		setupDebugging();

	window.createSurface(vkInstance, vkSurface);
	grabFirstPhysicalDevice();
	colorFormatAndSpace.init(vkPhysicalDevice, vkSurface);

	queueFamilies.init(vkPhysicalDevice);
	gfxQueueFamilyIndex = queueFamilies.getGfxCompXferQueues()[0];
	compQueueFamilyIndex = gfxQueueFamilyIndex;
	xferQueueFamilyIndex = queueFamilies.getTransferQueues()[0];

	createDevice();
	createQueues();
	createVmaAllocator();

	gpuBufferCache = std::make_unique<GpuBufferCache>(*this);
	renderPasses = renderPassCreator(vkDevice, colorFormatAndSpace);
	swapchain = Swapchain(vkDevice).replace(vkPhysicalDevice, vkDevice, window.getWidth(), window.getHeight(), vkSurface, colorFormatAndSpace);

	swapchainCreator.init(window);

	// where should this be initiated? need to try to get this into render thread
	commandPool = std::make_unique<CommandPool>(vkDevice);
	commandPool->initThread(*this);
	for (auto i = 0; i < FRAME_OVERLAP; i++)
		frameCmdBufs.push_back(commandPool->createGraphicsCmdBuf());
}

void VulkanContext::createVkInstance(bool validationOn) {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Demo";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "kengine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// must be done here to keep stuff in scope until vkInstance created
	// apply extensions
	auto glfwExtCnt = 0u;
	auto glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCnt);

	const char* myExts[] = {
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};
	const auto myExtCnt = sizeof(myExts) / sizeof(myExts[0]);

	auto allExts = std::vector<const char*>(glfwExts, glfwExts + glfwExtCnt);
	allExts.insert(allExts.end(), myExts, myExts + myExtCnt);

	if (allExts.size()) {
		createInfo.enabledExtensionCount = static_cast<uint32_t>(allExts.size());
		createInfo.ppEnabledExtensionNames = allExts.data();
	}

	// might change this logic since we might need layers that arent validation layers
	if (!validationOn) {
		VKCHECK(vkCreateInstance(&createInfo, nullptr, &vkInstance),
			"Failed to create Vulkan instance.");
		return;
	}

	// apply validation layers
	auto layerCount = 0u;
	VKCHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
		"Failed to get the number of instance layers.");

	auto availableLayers = std::vector<VkLayerProperties>(layerCount);

	VKCHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
		"Failed to get instance layers properties.");

	std::vector<const char*> desiredLayers = {
		  "VK_LAYER_LUNARG_standard_validation",
		  "VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> layersToEnable;

	// find layers that we want
	for (const auto& layerProperties : availableLayers) {
		if (std::any_of(desiredLayers.begin(), desiredLayers.end(),
			[&layerProperties](const char* desiredLayerName) {
				return strcmp(layerProperties.layerName, desiredLayerName) == 0;
			})) {
			layersToEnable.push_back(layerProperties.layerName);
		}
	}

	for (const auto& layerProperties : layersToEnable) {
		std::cout << "Enabled layer: " << layerProperties;
	}

	if (layersToEnable.size()) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(layersToEnable.size());
		createInfo.ppEnabledLayerNames = layersToEnable.data();
	}

	VKCHECK(vkCreateInstance(&createInfo, nullptr, &vkInstance),
		"Failed to create Vulkan instance.");
}


void VulkanContext::setupDebugging() {
	auto flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

	auto vkDebugCbCreateInfo = VkDebugReportCallbackCreateInfoEXT{};
	vkDebugCbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	vkDebugCbCreateInfo.flags = flags;
	vkDebugCbCreateInfo.pfnCallback = +[](VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData) -> VkBool32
		{
			std::cerr << "Validation Layer: " << msg << std::endl;
			return VK_FALSE;
		};

	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

	if (!func)
		throw std::runtime_error("Could not get address of vkCreateDebugReportCallbackEXT.");

	VkDebugReportCallbackEXT debugCallbackHandle;
	if (func(vkInstance, &vkDebugCbCreateInfo, nullptr, &debugCallbackHandle) != VK_SUCCESS)
		throw std::runtime_error("Failed to set up debug callback.");
}

void VulkanContext::grabFirstPhysicalDevice() {
	auto deviceCnt = 0u;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCnt, VK_NULL_HANDLE);

	if (!deviceCnt)
		throw std::runtime_error("No devices available.");

	std::vector<VkPhysicalDevice> availableDevices(deviceCnt);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCnt, availableDevices.data());

	for (const auto& device : availableDevices) {
		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(device, &props);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			vkPhysicalDevice = device;
			break;
		}
	}

	// load up gpu details
	vkPhysicalDeviceProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	vkGetPhysicalDeviceProperties2(vkPhysicalDevice, &vkPhysicalDeviceProps);

	// grab memory props
	vkPhysicalDeviceMemoryProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	vkGetPhysicalDeviceMemoryProperties2(vkPhysicalDevice, &vkPhysicalDeviceMemoryProps);
}

void VulkanContext::createDevice() {
	auto graphicsQueuePriority = 1.0f;
	auto graphicsQueueCreateInfo = VkDeviceQueueCreateInfo{};
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = gfxQueueFamilyIndex;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

	auto transferQueuePriority = 1.0f;
	auto transferQueueCreateInfo = VkDeviceQueueCreateInfo{};
	transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	transferQueueCreateInfo.queueFamilyIndex = xferQueueFamilyIndex;
	transferQueueCreateInfo.queueCount = 1;
	transferQueueCreateInfo.pQueuePriorities = &transferQueuePriority;

	auto features13 = VkPhysicalDeviceVulkan13Features{};
	features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features13.synchronization2 = true;
	features13.pNext = nullptr;

	auto features12 = VkPhysicalDeviceVulkan12Features{};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.pNext = &features13;

	auto features11 = VkPhysicalDeviceVulkan11Features{};
	features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	features11.pNext = &features12;

	VkDeviceQueueCreateInfo queueCreateInfos[2] = { graphicsQueueCreateInfo, transferQueueCreateInfo };

	std::vector<const char*> desiredLayers = {
		 VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		 //   VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
	};

	auto createDeviceInfo = VkDeviceCreateInfo{};
	createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createDeviceInfo.pNext = &features11;
	createDeviceInfo.queueCreateInfoCount = 2;
	createDeviceInfo.pQueueCreateInfos = queueCreateInfos;
	createDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(desiredLayers.size());
	createDeviceInfo.ppEnabledExtensionNames = desiredLayers.data();

	vkCreateDevice(vkPhysicalDevice, &createDeviceInfo, nullptr, &vkDevice);
}

void VulkanContext::createQueues() {
	graphicsQueue = std::make_shared<VulkanQueue>(vkDevice, gfxQueueFamilyIndex);
	computeQueue = graphicsQueue;
	transferQueue = std::make_unique<VulkanQueue>(vkDevice, xferQueueFamilyIndex);

	graphicsQueue->init();
	transferQueue->init();
}

void VulkanContext::createVmaAllocator() {
	auto allocatorInfo = VmaAllocatorCreateInfo{};
	allocatorInfo.instance = vkInstance;
	allocatorInfo.physicalDevice = vkPhysicalDevice;
	allocatorInfo.device = vkDevice;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	// required
	vmaVkFunctions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties");
	vmaVkFunctions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties");

	vmaVkFunctions.vkAllocateMemory = (PFN_vkAllocateMemory)vkGetDeviceProcAddr(vkDevice, "vkAllocateMemory");
	vmaVkFunctions.vkFreeMemory = (PFN_vkFreeMemory)vkGetDeviceProcAddr(vkDevice, "vkFreeMemory");
	vmaVkFunctions.vkMapMemory = (PFN_vkMapMemory)vkGetDeviceProcAddr(vkDevice, "vkMapMemory");
	vmaVkFunctions.vkUnmapMemory = (PFN_vkUnmapMemory)vkGetDeviceProcAddr(vkDevice, "vkUnmapMemory");
	vmaVkFunctions.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)vkGetDeviceProcAddr(vkDevice, "vkFlushMappedMemoryRanges");
	vmaVkFunctions.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)vkGetDeviceProcAddr(vkDevice, "vkInvalidateMappedMemoryRanges");
	vmaVkFunctions.vkBindBufferMemory = (PFN_vkBindBufferMemory)vkGetDeviceProcAddr(vkDevice, "vkBindBufferMemory");
	vmaVkFunctions.vkBindImageMemory = (PFN_vkBindImageMemory)vkGetDeviceProcAddr(vkDevice, "vkBindImageMemory");
	vmaVkFunctions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetBufferMemoryRequirements");
	vmaVkFunctions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetImageMemoryRequirements");
	vmaVkFunctions.vkCreateBuffer = (PFN_vkCreateBuffer)vkGetDeviceProcAddr(vkDevice, "vkCreateBuffer");
	vmaVkFunctions.vkDestroyBuffer = (PFN_vkDestroyBuffer)vkGetDeviceProcAddr(vkDevice, "vkDestroyBuffer");
	vmaVkFunctions.vkCreateImage = (PFN_vkCreateImage)vkGetDeviceProcAddr(vkDevice, "vkCreateImage");
	vmaVkFunctions.vkDestroyImage = (PFN_vkDestroyImage)vkGetDeviceProcAddr(vkDevice, "vkDestroyImage");
	vmaVkFunctions.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)vkGetDeviceProcAddr(vkDevice, "vkCmdCopyBuffer");

	//optional
	vmaVkFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties2");

	vmaVkFunctions.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(vkDevice, "vkGetBufferMemoryRequirements2");
	vmaVkFunctions.vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(vkDevice, "vkGetImageMemoryRequirements2");
	vmaVkFunctions.vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(vkDevice, "vkBindBufferMemory2");
	vmaVkFunctions.vkBindImageMemory2KHR = (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(vkDevice, "vkBindImageMemory2");
	vmaVkFunctions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetDeviceBufferMemoryRequirements");
	vmaVkFunctions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceProcAddr(vkDevice, "vkGetDeviceImageMemoryRequirements");

	allocatorInfo.pVulkanFunctions = &vmaVkFunctions;

	VKCHECK(vmaCreateAllocator(&allocatorInfo, &vmaAllocator),
		"Failed to initialize VMA allocator.");
}

VkDeviceSize VulkanContext::alignUboFrame(VkDeviceSize baseFrameSize) const {
	auto minSsboAlignment = vkPhysicalDeviceProps.properties.limits.minStorageBufferOffsetAlignment;

	if (minSsboAlignment <= 0)
		return baseFrameSize;

	return (baseFrameSize + minSsboAlignment - 1) & ~(minSsboAlignment - 1);
}

VkDeviceSize VulkanContext::alignSsboFrame(VkDeviceSize baseFrameSize) const {
	auto minSsboAlignment = vkPhysicalDeviceProps.properties.limits.minStorageBufferOffsetAlignment;

	if (minSsboAlignment <= 0)
		return baseFrameSize;

	return (baseFrameSize + minSsboAlignment - 1) & ~(minSsboAlignment - 1);
}

void SwapchainCreator::init(Window& window) {
	window.registerResizeListener([this](GLFWwindow* window, int newWidth, int newHeight) {
		std::unique_lock<std::mutex> lock(lock);
		targetWidth = newWidth;
		targetHeight = newHeight;
		setMustRecreate(true);
		});
}

bool SwapchainCreator::recreate(VulkanContext& vkCxt, bool force, Swapchain& oldSwapchain, OnSwapchainCreate& cb) {
	std::unique_lock<std::mutex> lock(lock);

	if (!mustRecreate && !force)
		return false;

	// await device idle
	VKCHECK(vkDeviceWaitIdle(vkCxt.getVkDevice()),
		"Failed to wait for device idle");

	// will destroy swapchain image views
	auto newSwapchain = oldSwapchain.replace(
		vkCxt.getVkPhysicalDevice(),
		vkCxt.getVkDevice(),
		targetWidth,
		targetHeight,
		vkCxt.getVkSurface(),
		vkCxt.getColorFormatAndSpace()
	);

	vkCxt.setSwapchain(std::move(newSwapchain));

	cb(vkCxt, *vkCxt.getSwapchain(), vkCxt.getRenderPasses());

	return true;
}

std::unique_ptr<GpuBuffer> VulkanContext::createBuffer(
	VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) const {

	VkBufferCreateInfo bufCreateInfo{};
	bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufCreateInfo.size = size;
	bufCreateInfo.usage = usageFlags;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = memoryUsage;
	allocationCreateInfo.flags = allocFlags;

	VkBuffer buffer{};
	VmaAllocation allocation{};
	VmaAllocationInfo allocationInfo{};
	VKCHECK(vmaCreateBuffer(vmaAllocator, &bufCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo),
		"Failed to allocate buffer.");

	VkMemoryPropertyFlags memTypeProperties{};
	vmaGetMemoryTypeProperties(vmaAllocator, allocationInfo.memoryType, &memTypeProperties);
	auto isHostCoherent = (memTypeProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

	return std::make_unique<GpuBuffer>(vmaAllocator, buffer, allocation, isHostCoherent);
}

void VulkanContext::submitQueueTransfer(std::shared_ptr<QueueOwnerTransfer> qXfer) {
	std::lock_guard<std::mutex> lock(qXferMtx);
	vkQueueTransfers.push(qXfer);
}

void VulkanContext::uploadBuffer(GpuUploadable& obj, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
	VkBufferUsageFlags usageFlags, std::function<void(VkCommandBuffer)> beforeSubmit) {
	uploadBuffer(obj, dstStageMask, dstAccessMask, usageFlags, 0, beforeSubmit);
}

void VulkanContext::uploadBuffer(GpuUploadable& obj, VkAccessFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
	VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocFlags, std::function<void(VkCommandBuffer)> beforeSubmit) {
	// create staging buffer
	auto stagingBuf = createBuffer(
		obj.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
	);

	// copy to staging buffer
	{
		GpuBuffer::ScopedMap scopedMap(*stagingBuf);
		obj.upload(*this, stagingBuf->data());
	}

	// create device buffer
	auto deviceBuf = createBuffer(
		obj.size(),
		usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		allocFlags
	);

	auto cmdBuf = commandPool->createTransferCmdBuf();

	// record and submit to queue, and block until queue work finish
	// share sStagingBuf with lambda so it'll be in respective scopes
	std::shared_ptr<GpuBuffer> sStagingBuf = std::move(stagingBuf);
	recordAndSubmitCmdBuf(
		std::move(cmdBuf),
		*transferQueue,
		[&](const CommandBuffer& cmdBuf) {
			auto qXfer = std::make_shared<BufferQueueOwnerTransfer>(
				deviceBuf->getVkBuffer(), obj.size(),
				xferQueueFamilyIndex, gfxQueueFamilyIndex,
				dstStageMask, dstAccessMask
			);
			qXfer->applyReleaseBarrier(cmdBuf.vkCmdBuf);

			VkBufferCopy copy{ 0, 0, obj.size() };
			vkCmdCopyBuffer(cmdBuf.vkCmdBuf, sStagingBuf->vkBuffer, deviceBuf->vkBuffer, 1, &copy);

			if (beforeSubmit)
				beforeSubmit(cmdBuf.vkCmdBuf);

			// executes on fence signaled
			return [this, sStagingBuf, qXfer]() {
				this->submitQueueTransfer(qXfer);
				};
		},
		true);

	// transfer device buffer ownershiop to uploadable obj
	obj.setGpuBuffer(std::move(deviceBuf));
}

void VulkanContext::recordAndSubmitCmdBuf(std::unique_ptr<CommandBuffer>&& cmd, VulkanQueue& queue, CommandBufferRecordFunc func, bool awaitFence) {
	// record command buffer
	VkCommandBufferBeginInfo cmdBufBeginInfo{};
	cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VKCHECK(vkBeginCommandBuffer(cmd->vkCmdBuf, &cmdBufBeginInfo),
		"Failed to begin command buffer.");

	auto followUp = func(*cmd);

	VKCHECK(vkEndCommandBuffer(cmd->vkCmdBuf),
		"Failed to end command buffer.");

	// submit command buffer
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	VKCHECK(vkCreateFence(vkDevice, &fenceCreateInfo, VK_NULL_HANDLE, &fence),
		"Failed to create fence.");

	VkCommandBufferSubmitInfo cmdBufSubmitInfo{};
	cmdBufSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmdBufSubmitInfo.commandBuffer = cmd->vkCmdBuf;

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdBufSubmitInfo;

	VKCHECK(queue.submit(1, &submitInfo, fence),
		"Failed to submit command buffer.");

	if (!awaitFence) {
		std::lock_guard<std::mutex> lock(waitingFenceMtx);
		// function lambda executes a copy, and we cant copy a unique ptr...
		std::shared_ptr<CommandBuffer> sCmd = std::move(cmd);
		vkFenceActions[fence] = [sCmd, followUp]() mutable {
			// cmd.dispose(); // smart ptr takes care of this for us
			if (followUp)
				followUp();
			};

		return;
	}

	vkWaitForFences(vkDevice, 1, &fence, true, LLONG_MAX);

	if (followUp)
		followUp();
}