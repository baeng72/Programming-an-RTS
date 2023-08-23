#include "Vulkan.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>


#ifdef __USE__VMA__
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulkan{
#if !defined NDEBUG
	//https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Validation_layers
	VkDebugUtilsMessengerEXT debugMessenger;
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void setupDebugMessenger(VkInstance instance) {


		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			assert("failed to set up debug messenger!");
		}
	}
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

#endif
#ifdef __USE__VMA__
	VkInstance allocatorInstance{ VK_NULL_HANDLE };
	VmaAllocator allocator{ VK_NULL_HANDLE };
#endif
	VkInstance initInstance(std::vector<const char*>& requiredExtensions, std::vector<const char*>& requiredLayers) {
#ifdef __USE__VOLK__
		VkResult  res = volkInitialize();
		assert(res == VK_SUCCESS);
#else
		VkResult res;
#endif
		VkInstance instance{ VK_NULL_HANDLE };
		auto first = requiredExtensions.cbegin();
		auto last = requiredExtensions.cend();
		bool found = false;
		while (first != last) {
			if (_strcmpi(*first, "VK_KHR_surface") == 0) {
				found = true;
				break;
			}
			++first;
		}
		if (!found) {
			requiredExtensions.push_back("VK_KHR_surface");
		}
		found = false;
#ifdef WIN32
		first = requiredExtensions.cbegin();

		while (first != last) {

			if (_strcmpi(*first, "VK_KHR_win32_surface") == 0) {
				found = true;
				break;
			}
			++first;
		}

		if (!found) {
			requiredExtensions.push_back("VK_KHR_win32_surface");
		}
#endif
		VkApplicationInfo appInfo{};
		appInfo.apiVersion = VK_API_VERSION_1_0;
		appInfo.applicationVersion = 1;
		appInfo.pApplicationName = "VulkEng";
		appInfo.pEngineName = "VulkEng";
		VkInstanceCreateInfo instanceCI{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instanceCI.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		instanceCI.ppEnabledExtensionNames = requiredExtensions.data();
		instanceCI.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		instanceCI.ppEnabledLayerNames = requiredLayers.data();
		instanceCI.pApplicationInfo = &appInfo;
		res = vkCreateInstance(&instanceCI, nullptr, &instance);
		assert(res == VK_SUCCESS);
		assert(instance != VK_NULL_HANDLE);
#ifdef __USE__VMA__
		allocatorInstance = instance;
#endif
#ifdef __USE__VOLK__
		volkLoadInstance(instance);
#endif
#if !defined NDEBUG
		setupDebugMessenger(instance);
#endif
		return instance;
	}

	void cleanupInstance(VkInstance instance) {
#if !defined NDEBUG
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
		vkDestroyInstance(instance, nullptr);
	}

	VkSurfaceKHR initSurfaceGLFW(VkInstance instance, GLFWwindow* window) {
		VkSurfaceKHR surface{ VK_NULL_HANDLE };
		VkResult res = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		assert(res == VK_SUCCESS);
		return surface;
	}

	void cleanupSurface(VkInstance instance, VkSurfaceKHR surface) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	VkPhysicalDevice choosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, Queues& queues) {
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		uint32_t physicalDeviceCount = 0;
		VkResult res;
		res = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		assert(res == VK_SUCCESS);
		assert(physicalDeviceCount > 0);
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		res = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
		assert(res == VK_SUCCESS);


		for (size_t i = 0; i < physicalDevices.size(); i++) {
			VkPhysicalDevice phys = physicalDevices[i];
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(phys, &physicalDeviceProperties);

			if (physicalDeviceProperties.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				physicalDevice = physicalDevices[i];
				break;
			}
		}
		assert(physicalDevice != VK_NULL_HANDLE);
		uint32_t graphicsQueueFamily = UINT32_MAX;
		uint32_t presentQueueFamily = UINT32_MAX;
		uint32_t computeQueueFamily = UINT32_MAX;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());


		for (uint32_t i = 0; i < queueFamilyCount; i++) {
			VkBool32 supportsPresent = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent);
			VkQueueFamilyProperties& queueProps = queueFamilyProperties[i];
			if (graphicsQueueFamily == UINT32_MAX && queueProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsQueueFamily = i;
			if (presentQueueFamily == UINT32_MAX && supportsPresent)
				presentQueueFamily = i;
			if (computeQueueFamily == UINT32_MAX && queueProps.queueFlags & VK_QUEUE_COMPUTE_BIT)
				computeQueueFamily = i;
			if (graphicsQueueFamily != UINT32_MAX && presentQueueFamily != UINT32_MAX && computeQueueFamily != UINT32_MAX)
				break;
		}
		assert(graphicsQueueFamily != UINT32_MAX && presentQueueFamily != UINT32_MAX && computeQueueFamily != UINT32_MAX);
		assert(computeQueueFamily == graphicsQueueFamily && graphicsQueueFamily == presentQueueFamily);//support one queue for now	
		queues.graphicsQueueFamily = graphicsQueueFamily;
		queues.presentQueueFamily = presentQueueFamily;
		queues.computeQueueFamily = computeQueueFamily;
		return physicalDevice;
	}

	bool supportsDeviceExtension(VkPhysicalDevice physicalDevice, const char* pExtension) {

		uint32_t count = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> properties(count);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, properties.data());
		for (auto& ext : properties) {
			if (!_strcmpi(ext.extensionName, pExtension))
				return true;
		}
		return false;
	}

	VkDevice initDevice(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensions, Queues queues, VkPhysicalDeviceFeatures enabledFeatures, uint32_t queueCount) {
		VkDevice device{ VK_NULL_HANDLE };
		std::vector<float> queuePriorities(queueCount, 1.0f);
		std::vector<VkDeviceQueueCreateInfo> queueCIs;

		if (queues.computeQueueFamily == queues.graphicsQueueFamily && queues.graphicsQueueFamily == queues.presentQueueFamily) {
			//queuePriorities.push_back(1.0f);
			queueCIs.push_back({ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,nullptr,0,queues.graphicsQueueFamily,queueCount,queuePriorities.data() });
		}
		else {
			//shouldn't get here for now
		}

		VkDeviceCreateInfo deviceCI{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCI.ppEnabledExtensionNames = deviceExtensions.data();
		deviceCI.pEnabledFeatures = &enabledFeatures;
		deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size());
		deviceCI.pQueueCreateInfos = queueCIs.data();
		VkResult res = vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device);
		assert(res == VK_SUCCESS);
#ifdef __USE__VMA__
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;//will this clash if device is version 1.0?
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = allocatorInstance;
#ifdef __USE__VOLK__
		//https ://stackoverflow.com/questions/73512602/using-vulkan-memory-allocator-with-volk
		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
		vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
		vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
		vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
		vma_vulkan_func.vkCreateImage = vkCreateImage;
		vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
		vma_vulkan_func.vkDestroyImage = vkDestroyImage;
		vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vma_vulkan_func.vkFreeMemory = vkFreeMemory;
		vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vma_vulkan_func.vkMapMemory = vkMapMemory;
		vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
		vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		allocatorInfo.pVulkanFunctions = &vma_vulkan_func;
#endif
		res = vmaCreateAllocator(&allocatorInfo, &allocator);
		assert(res == VK_SUCCESS);
		assert(allocator != VK_NULL_HANDLE);
#endif
#ifdef __USE__VOLK__
		//volkLoadDevice(device);
#endif

		//initShaderModule(device, "assets/shaders/pbr-notex.vert.spv");
		return device;
	}

	void cleanupDevice(VkDevice device) {
#ifdef __USE__VMA__
		vmaDestroyAllocator(allocator);
#endif 
		vkDestroyDevice(device, nullptr);
	}

	VkQueue getDeviceQueue(VkDevice device, uint32_t queueFamily, uint32_t queueIndex) {
		VkQueue queue{ VK_NULL_HANDLE };
		vkGetDeviceQueue(device, queueFamily, queueIndex, &queue);
		assert(queue);
		return queue;
	}

	VkPresentModeKHR chooseSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes) {
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (size_t i = 0; i < presentModes.size(); i++) {
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = presentModes[i];
				break;
			}
			if ((presentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
				presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
		return presentMode;
	}

	VkSurfaceFormatKHR chooseSwapchainFormat(std::vector<VkSurfaceFormatKHR>& formats, VkFormat preferredFormat) {
		VkSurfaceFormatKHR format;
		if (formats.size() > 0)
			format = formats[0];

		for (auto&& surfaceFormat : formats) {
			if (surfaceFormat.format == preferredFormat) {
				format = surfaceFormat;
				break;
			}
		}
		return format;
	}

	VkSurfaceTransformFlagsKHR chooseSwapchainTransform(VkSurfaceCapabilitiesKHR& surfaceCaps) {

		VkSurfaceTransformFlagsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		if (!(surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR))
			transform = surfaceCaps.currentTransform;
		return transform;
	}

	VkCompositeAlphaFlagBitsKHR chooseSwapchainComposite(VkSurfaceCapabilitiesKHR& surfaceCaps) {
		VkCompositeAlphaFlagBitsKHR compositeFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
				VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (surfaceCaps.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeFlags = compositeAlphaFlag;
				break;
			};
		}
		return compositeFlags;
	}

	VkSemaphore initSemaphore(VkDevice device) {
		VkSemaphore semaphore{ VK_NULL_HANDLE };
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkResult res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore);
		assert(res == VK_SUCCESS);
		return semaphore;
	}

	void cleanupSemaphore(VkDevice device, VkSemaphore semaphore) {
		vkDestroySemaphore(device, semaphore, nullptr);
	}


	VkCommandPool initCommandPool(VkDevice device, uint32_t queueFamily) {
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		VkCommandPoolCreateInfo cmdPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		cmdPoolCI.queueFamilyIndex = queueFamily;
		cmdPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VkResult res = vkCreateCommandPool(device, &cmdPoolCI, nullptr, &commandPool);
		assert(res == VK_SUCCESS);
		return commandPool;
	}

	void initCommandPools(VkDevice device, size_t size, uint32_t queueFamily, std::vector<VkCommandPool>& commandPools) {
		commandPools.resize(size, VK_NULL_HANDLE);
		for (size_t i = 0; i < size; i++) {
			VkCommandPoolCreateInfo cmdPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			cmdPoolCI.queueFamilyIndex = queueFamily;
			cmdPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VkResult res = vkCreateCommandPool(device, &cmdPoolCI, nullptr, &commandPools[i]);
			assert(res == VK_SUCCESS);
		}
	}

	void initCommandPools(VkDevice device, size_t size, uint32_t queueFamily, VkCommandPool* commandPools) {
		
		for (size_t i = 0; i < size; i++) {
			VkCommandPoolCreateInfo cmdPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			cmdPoolCI.queueFamilyIndex = queueFamily;
			cmdPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VkResult res = vkCreateCommandPool(device, &cmdPoolCI, nullptr, &commandPools[i]);
			assert(res == VK_SUCCESS);
		}
	}


	VkCommandBuffer initCommandBuffer(VkDevice device, VkCommandPool commandPool) {
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		VkCommandBufferAllocateInfo cmdBufAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		cmdBufAI.commandPool = commandPool;
		cmdBufAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAI.commandBufferCount = 1;
		VkResult res = vkAllocateCommandBuffers(device, &cmdBufAI, &commandBuffer);
		assert(res == VK_SUCCESS);


		return commandBuffer;
	}

	void initCommandBuffers(VkDevice device, std::vector<VkCommandPool>& commandPools, std::vector<VkCommandBuffer>& commandBuffers) {
		commandBuffers.resize(commandPools.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < commandPools.size(); i++) {
			VkCommandBufferAllocateInfo cmdBufAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			cmdBufAI.commandPool = commandPools[i];
			cmdBufAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAI.commandBufferCount = 1;
			VkResult res = vkAllocateCommandBuffers(device, &cmdBufAI, &commandBuffers[i]);
			assert(res == VK_SUCCESS);
		}
	}
	void initCommandBuffers(VkDevice device, VkCommandPool* commandPools, VkCommandBuffer* commandBuffers,uint32_t count) {
		
		for (uint32_t i = 0; i < count; i++) {
			VkCommandBufferAllocateInfo cmdBufAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			cmdBufAI.commandPool = commandPools[i];
			cmdBufAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAI.commandBufferCount = 1;
			VkResult res = vkAllocateCommandBuffers(device, &cmdBufAI, &commandBuffers[i]);
			assert(res == VK_SUCCESS);
		}
	}

	void cleanupCommandBuffers(VkDevice device, std::vector<VkCommandPool>& commandPools, std::vector<VkCommandBuffer>& commandBuffers) {
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			vkFreeCommandBuffers(device, commandPools[i], 1, &commandBuffers[i]);
		}
	}

	void cleanupCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer) {
		std::vector<VkCommandPool> commandPools{ commandPool };
		std::vector<VkCommandBuffer> commandBuffers{ commandBuffer };
		cleanupCommandBuffers(device, commandPools, commandBuffers);
	}



	void cleanupCommandPools(VkDevice device, std::vector<VkCommandPool>& commandPools) {
		for (auto& commandPool : commandPools) {
			vkDestroyCommandPool(device, commandPool, nullptr);
		}
	}

	void cleanupCommandPool(VkDevice device, VkCommandPool commandPool) {
		std::vector<VkCommandPool> commandPools{ commandPool };
		cleanupCommandPools(device, commandPools);
	}

	VkSwapchainKHR initSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR& surfaceCaps, VkPresentModeKHR& presentMode, VkSurfaceFormatKHR& swapchainFormat, VkExtent2D& swapchainExtent, uint32_t imageCount, VkSwapchainKHR oldSwapchain) {
		VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

		VkSurfaceTransformFlagsKHR preTransform = chooseSwapchainTransform(surfaceCaps);
		VkCompositeAlphaFlagBitsKHR compositeAlpha = chooseSwapchainComposite(surfaceCaps);

		if (surfaceCaps.currentExtent.width == (uint32_t)-1 || oldSwapchain != VK_NULL_HANDLE) {
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else {
			swapchainExtent = surfaceCaps.currentExtent;
		}

		uint32_t desiredNumberOfSwapchainImages = imageCount == UINT32_MAX ? surfaceCaps.minImageCount + 1 : imageCount;
		if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainCI = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

		swapchainCI.surface = surface;
		swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
		swapchainCI.imageFormat = swapchainFormat.format;
		swapchainCI.imageColorSpace = swapchainFormat.colorSpace;
		swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCI.queueFamilyIndexCount = 0;
		swapchainCI.pQueueFamilyIndices = nullptr;
		swapchainCI.presentMode = presentMode;
		swapchainCI.oldSwapchain = oldSwapchain;
		swapchainCI.clipped = VK_TRUE;
		swapchainCI.compositeAlpha = compositeAlpha;
		if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Enable transfer destination on swap chain images if supported
		if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		VkResult res = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain);
		assert(res == VK_SUCCESS);
		return swapchain;
	}

	void getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& images) {
		uint32_t imageCount = 0;
		VkResult res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		assert(res == VK_SUCCESS);
		assert(imageCount > 0);
		images.resize(imageCount);
		res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
		assert(res == VK_SUCCESS);
	}
	void getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, VkImage* images,uint32_t&maxCount) {
		uint32_t imageCount = 0;
		VkResult res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		assert(res == VK_SUCCESS);
		assert(imageCount > 0);
		assert(imageCount <= maxCount);
		res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images);
		assert(res == VK_SUCCESS);
		maxCount = imageCount;
	}

	uint32_t getSwapchainImageCount(VkSurfaceCapabilitiesKHR& surfaceCaps) {
		uint32_t desiredNumberOfSwapchainImages = surfaceCaps.minImageCount + 1;
		if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
		}
		return desiredNumberOfSwapchainImages;
	}

	void initSwapchainImageViews(VkDevice device, std::vector<VkImage>& swapchainImages, VkFormat& swapchainFormat, std::vector<VkImageView>& swapchainImageViews) {
		swapchainImageViews.resize(swapchainImages.size());
		for (size_t i = 0; i < swapchainImages.size(); i++) {
			VkImageViewCreateInfo viewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewCI.format = swapchainFormat;
			viewCI.components = { VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };
			viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//attachment/view with be color
			viewCI.subresourceRange.baseMipLevel = 0;
			viewCI.subresourceRange.levelCount = 1;
			viewCI.subresourceRange.baseArrayLayer = 0;
			viewCI.subresourceRange.layerCount = 1;
			viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCI.image = swapchainImages[i];
			VkResult res = vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]);
			assert(res == VK_SUCCESS);
		}
	}
	void initSwapchainImageViews(VkDevice device, VkImage* swapchainImages, VkFormat& swapchainFormat, VkImageView* swapchainImageViews,uint32_t imageCount) {
	
		for (size_t i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo viewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewCI.format = swapchainFormat;
			viewCI.components = { VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };
			viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//attachment/view with be color
			viewCI.subresourceRange.baseMipLevel = 0;
			viewCI.subresourceRange.levelCount = 1;
			viewCI.subresourceRange.baseArrayLayer = 0;
			viewCI.subresourceRange.layerCount = 1;
			viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCI.image = swapchainImages[i];
			VkResult res = vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]);
			assert(res == VK_SUCCESS);
		}
	}
	void cleanupSwapchainImageViews(VkDevice device, std::vector<VkImageView>& imageViews) {
		for (auto& imageView : imageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
	}
	void cleanupSwapchainImageViews(VkDevice device, VkImageView* imageViews,uint32_t count) {
		for (uint32_t i = 0; i < count;i++) {
			vkDestroyImageView(device, imageViews[i], nullptr);
		}
	}
	void cleanupSwapchain(VkDevice device, VkSwapchainKHR swapchain) {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}






	VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDeviceProperties deviceProperties) {



		VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts & deviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	inline uint32_t getMipLevels(uint32_t width, uint32_t height) {
		uint32_t m = std::max(width, height);
		uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
		return mipLevels;
	}
	inline uint32_t getMipLevels(ImageProperties& image) {
		uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;
		return mipLevels;
	}

	void initTexture(VkDevice device, VkPhysicalDeviceMemoryProperties& memoryProperties, TextureProperties& props, Texture& texture) {
		initImage(device, memoryProperties, props, texture);

		initSampler(device, props.samplerProps, texture);

	}

	void initImage(VkDevice device, VkImageCreateInfo& imageCI, Image& image, bool isMapped) {
#ifdef __USE__VMA__
		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = isMapped ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;//might be wrong?
		imageAllocCreateInfo.flags = isMapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
		VkResult res = vmaCreateImage(allocator, &imageCI, &imageAllocCreateInfo, &image.image, &image.allocation, &image.allocationInfo);
		assert(res == VK_SUCCESS);
#else
		VkResult res = vkCreateImage(device, &imageCI, nullptr, &image.image);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(device, image.image, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, memoryProperties, props.memoryProps);
		res = vkAllocateMemory(device, &memAllocInfo, nullptr, &image.memory);
		assert(res == VK_SUCCESS);
		res = vkBindImageMemory(device, image.image, image.memory, 0);
		assert(res == VK_SUCCESS);
#endif


		image.width = imageCI.extent.width;
		image.height = imageCI.extent.height;
		image.mipLevels = 1;

	}

	void initImage(VkDevice device, VkPhysicalDeviceMemoryProperties& memoryProperties, ImageProperties& props, Image& image) {
		uint32_t mipLevels = props.mipLevels == 0 ? getMipLevels(props) : props.mipLevels;
		VkImageCreateInfo imageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = props.format;
		imageCI.extent = { props.width,props.height,1 };
		imageCI.mipLevels = mipLevels;
		imageCI.arrayLayers = props.layers;
		imageCI.samples = props.samples;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.initialLayout = props.layout;// VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.usage = props.imageUsage;
		if (props.isCubeMap)
			imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;


#ifdef __USE__VMA__
		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;//might be wrong?
		VkResult res = vmaCreateImage(allocator, &imageCI, &imageAllocCreateInfo, &image.image, &image.allocation, &image.allocationInfo);
		assert(res == VK_SUCCESS);

#else
		VkResult res = vkCreateImage(device, &imageCI, nullptr, &image.image);
		assert(res == VK_SUCCESS);

		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(device, image.image, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, memoryProperties, props.memoryProps);
		res = vkAllocateMemory(device, &memAllocInfo, nullptr, &image.memory);
		assert(res == VK_SUCCESS);
		res = vkBindImageMemory(device, image.image, image.memory, 0);
		assert(res == VK_SUCCESS);
#endif
		VkImageViewCreateInfo imageViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		imageViewCI.viewType = props.layers > 1 ? (props.isCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D_ARRAY) : VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = props.format;
		imageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		imageViewCI.subresourceRange = { props.aspect,0,1,0,1 };
		imageViewCI.image = image.image;
		imageViewCI.subresourceRange.levelCount = mipLevels;
		imageViewCI.subresourceRange.layerCount = props.layers;



		res = vkCreateImageView(device, &imageViewCI, nullptr, &image.imageView);
		assert(res == VK_SUCCESS);

		image.width = props.width;
		image.height = props.height;
		image.mipLevels = mipLevels;
		image.layerCount = props.layers;
	}

	void initSampler(VkDevice device, SamplerProperties& props, Texture& texture) {
		VkSamplerCreateInfo samplerCI{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerCI.magFilter = samplerCI.minFilter = props.filter;// VK_FILTER_LINEAR;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = samplerCI.addressModeV = samplerCI.addressModeW = props.addressMode;// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)texture.mipLevels;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VkResult res = vkCreateSampler(device, &samplerCI, nullptr, &texture.sampler);
		assert(res == VK_SUCCESS);
	}
	VkSampler initSampler(VkDevice device, SamplerProperties& props, uint32_t mipLevels) {
		VkSamplerCreateInfo samplerCI{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerCI.magFilter = samplerCI.minFilter = props.filter;// VK_FILTER_LINEAR;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = samplerCI.addressModeV = samplerCI.addressModeW = props.addressMode;// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (float)mipLevels;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VkSampler sampler;
		VkResult res = vkCreateSampler(device, &samplerCI, nullptr, &sampler);
		assert(res == VK_SUCCESS);
		return sampler;
	}

	VkFormat getSupportedDepthFormat(VkPhysicalDevice physicalDevice) {
		// Since all depth formats may be optional, we need to find a suitable depth format to use
			// Start with the highest precision packed format
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
			// Format must support depth stencil attachment for optimal tiling
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				return format;
			}
		}

		return VK_FORMAT_UNDEFINED;
	}

	void generateMipMaps(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Image& image) {
		//create mip maps
		uint32_t mipLevels = image.mipLevels;
		uint32_t layerCount = image.layerCount;
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
		assert(res == VK_SUCCESS);
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image.image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = image.width;
		int32_t mipHeight = image.height;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = layerCount;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = layerCount;

			vkCmdBlitImage(cmd,
				image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);


			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkFence fence = initFence(device);


		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(res == VK_SUCCESS);

		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(res == VK_SUCCESS);

		vkDestroyFence(device, fence, nullptr);

	}

	void CopyBufferToImage(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Buffer& src, Image& dst, uint32_t width, uint32_t height, VkDeviceSize offset, uint32_t arrayLayer) {
		VkBufferCopy copyRegion = {};
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
		assert(res == VK_SUCCESS);

		VkBufferImageCopy region{};
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageExtent = { width,height,1 };
		region.bufferOffset = offset;
		region.imageSubresource.baseArrayLayer = arrayLayer;
		vkCmdCopyBufferToImage(cmd,
			src.buffer,
			dst.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkFence fence = initFence(device);


		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(res == VK_SUCCESS);

		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(res == VK_SUCCESS);


		vkDestroyFence(device, fence, nullptr);
	}
	void CopyBufferToImage(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Buffer& src, Image& dst, std::vector<VkBufferImageCopy>& copyRegions) {
		VkBufferCopy copyRegion = {};
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
		assert(res == VK_SUCCESS);


		vkCmdCopyBufferToImage(cmd,
			src.buffer,
			dst.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			(uint32_t)copyRegions.size(),
			copyRegions.data());
		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkFence fence = initFence(device);


		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(res == VK_SUCCESS);

		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(res == VK_SUCCESS);


		vkDestroyFence(device, fence, nullptr);
	}
	void transitionImageNoSubmit(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, uint32_t layerIndex) {
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = layerIndex;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		switch (oldLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
		}
		switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
		}
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {

		}
		vkCmdPipelineBarrier(cmd, sourceStage, destinationStage,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

	}

	void transitionImage(VkDevice device, VkQueue queue, VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, uint32_t layerIndex) {
		//VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		//barrier.oldLayout = oldLayout;
		//barrier.newLayout = newLayout;
		//barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//barrier.image = image;
		//barrier.subresourceRange.baseMipLevel = 0;
		//barrier.subresourceRange.levelCount = mipLevels;
		//barrier.subresourceRange.baseArrayLayer = layerIndex;
		//barrier.subresourceRange.layerCount = layerCount;
		//barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		//VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		//switch (oldLayout) {
		//case VK_IMAGE_LAYOUT_UNDEFINED:
		//	barrier.srcAccessMask = 0;
		//	break;
		//case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		//	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		//	barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		//	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		//	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		//	barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//	break;
		//default:
		//	break;
		//}
		//switch (newLayout) {
		//case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		//	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		//	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		//	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//	break;
		//case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		//	// Image layout will be used as a depth/stencil attachment
		//	// Make sure any writes to depth/stencil buffer have been finished
		//	barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		//	break;

		//case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		//	// Image will be read in a shader (sampler, input attachment)
		//	// Make sure any writes to the image have been finished
		//	if (barrier.srcAccessMask == 0)
		//	{
		//		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		//	}
		//	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//	break;
		//default:
		//	break;
		//}
		//if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		//	barrier.srcAccessMask = 0;
		//	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//	sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		//	destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		//}
		//else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		//	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//	sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		//	destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		//}
		//else {

		//}
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
		assert(res == VK_SUCCESS);
		/*	vkCmdPipelineBarrier(cmd, sourceStage, destinationStage,
				0, 0, nullptr, 0, nullptr, 1, &barrier);*/
		transitionImageNoSubmit(cmd, image, oldLayout, newLayout, mipLevels, layerCount, layerIndex);
		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);


		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkFence fence = initFence(device);


		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(res == VK_SUCCESS);

		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(res == VK_SUCCESS);


		vkDestroyFence(device, fence, nullptr);
	}


	void cleanupTexture(VkDevice device, Texture& texture) {
		if (texture.sampler != VK_NULL_HANDLE)
			vkDestroySampler(device, texture.sampler, nullptr);
		cleanupImage(device, texture);
	}

	void cleanupSampler(VkDevice device_, VkSampler sampler_) {
		vkDestroySampler(device_, sampler_, nullptr);
	}

	void cleanupImage(VkDevice device, Image& image) {

		if (image.imageView != VK_NULL_HANDLE)
			vkDestroyImageView(device, image.imageView, nullptr);
#ifdef __USE__VMA__
		if (image.allocation != VK_NULL_HANDLE) {
			vmaDestroyImage(allocator, image.image, image.allocation);
		}
#else
		if (image.memory != VK_NULL_HANDLE)
			vkFreeMemory(device, image.memory, nullptr);
		if (image.image != VK_NULL_HANDLE)
			vkDestroyImage(device, image.image, nullptr);
#endif

	}

	VkRenderPass initRenderPass(VkDevice device, RenderPassProperties& props) {//VkFormat colorFormat, VkSampleCountFlagBits numSamples) {

		VkRenderPass renderPass{ VK_NULL_HANDLE };
		std::vector< VkAttachmentDescription> attachments;
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		VkAttachmentDescription colorAttachment{};
		VkAttachmentReference colorRef = { 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		if (props.colorFormat != VK_FORMAT_UNDEFINED) {
			colorAttachment.format = props.colorFormat;
			colorAttachment.samples = props.sampleCount;
			colorAttachment.loadOp = props.colorLoadOp;// VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = props.colorStoreOp;// VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = props.colorInitialLayout;
			colorAttachment.finalLayout = props.colorFinalLayout;// presentPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorRef;
			attachments.push_back(colorAttachment);
		}


		VkAttachmentDescription depthAttachment{};
		VkAttachmentReference depthAttachmentRef{};
		if (props.depthFormat != VK_FORMAT_UNDEFINED) {
			depthAttachment.format = props.depthFormat;
			depthAttachment.samples = props.sampleCount;
			depthAttachment.loadOp = props.depthLoadOp;// VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = props.depthStoreOp; //VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = props.depthInitialLayout;// VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = props.depthFinalLayout;// VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentRef.attachment = (uint32_t)attachments.size();
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
			attachments.push_back(depthAttachment);
		}



		VkAttachmentDescription colorAttachmentResolve{};
		VkAttachmentReference colorAttachmentResolveRef{};
		if (props.resolveFormat != VK_FORMAT_UNDEFINED) {
			colorAttachmentResolve.format = props.resolveFormat;// colorFormat;
			colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			colorAttachmentResolveRef.attachment = (uint32_t)attachments.size();
			colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			subpass.pResolveAttachments = &colorAttachmentResolveRef;
			attachments.push_back(colorAttachmentResolve);
		}
		VkSubpassDependency dependency{};
		std::vector<VkSubpassDependency> dependencies;
		if (props.dependencies.size() == 0) {
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies.push_back(dependency);
		}
		else
			dependencies = props.dependencies;
		//VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment,colorAttachmentResolve };
		VkRenderPassCreateInfo renderCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderCI.attachmentCount = (uint32_t)attachments.size();
		renderCI.pAttachments = attachments.data();
		renderCI.subpassCount = 1;
		renderCI.pSubpasses = &subpass;
		renderCI.dependencyCount = (uint32_t)dependencies.size();
		renderCI.pDependencies = dependencies.data();

		VkResult res = vkCreateRenderPass(device, &renderCI, nullptr, &renderPass);
		assert(res == VK_SUCCESS);

		return renderPass;
	}

	void cleanupRenderPass(VkDevice device, VkRenderPass renderPass) {
		vkDestroyRenderPass(device, renderPass, nullptr);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkPhysicalDeviceMemoryProperties memoryProperties, VkMemoryPropertyFlags properties) {
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		assert(0);
		return 0;
	}

	void initFramebuffers(VkDevice device, VkRenderPass renderPass, FramebufferProperties& fbProps, std::vector<VkFramebuffer>& framebuffers) {
		size_t size = fbProps.colorAttachmentCount > 0 ? fbProps.colorAttachmentCount : 1;
		framebuffers.resize(size, VK_NULL_HANDLE);
		for (size_t i = 0; i < size; i++) {
			std::vector<VkImageView> attachments;
			if (fbProps.resolveAttachment != VK_NULL_HANDLE)
				attachments.push_back(fbProps.resolveAttachment);
			else if (fbProps.colorAttachmentCount > 0)
				attachments.push_back(fbProps.colorAttachments[i]);
			if (fbProps.depthAttachment != VK_NULL_HANDLE)
				attachments.push_back(fbProps.depthAttachment);
			if (fbProps.resolveAttachment != VK_NULL_HANDLE)
				attachments.push_back(fbProps.colorAttachments[i]);

			VkFramebufferCreateInfo fbCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			fbCI.renderPass = renderPass;
			fbCI.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbCI.pAttachments = attachments.data();
			fbCI.width = fbProps.width;
			fbCI.height = fbProps.height;
			fbCI.layers = 1;
			VkResult res = vkCreateFramebuffer(device, &fbCI, nullptr, &framebuffers[i]);
			assert(res == VK_SUCCESS);
		}
	}


	void initFramebuffers(VkDevice device, VkRenderPass renderPass, FramebufferProperties& fbProps, VkFramebuffer * framebuffers) {
		size_t size = fbProps.colorAttachmentCount > 0 ? fbProps.colorAttachmentCount : 1;
		
		for (size_t i = 0; i < size; i++) {
			std::vector<VkImageView> attachments;
			if (fbProps.resolveAttachment != VK_NULL_HANDLE)
				attachments.push_back(fbProps.resolveAttachment);
			else if (fbProps.colorAttachmentCount > 0)
				attachments.push_back(fbProps.colorAttachments[i]);
			if (fbProps.depthAttachment != VK_NULL_HANDLE)
				attachments.push_back(fbProps.depthAttachment);
			if (fbProps.resolveAttachment != VK_NULL_HANDLE)
				attachments.push_back(fbProps.colorAttachments[i]);

			VkFramebufferCreateInfo fbCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			fbCI.renderPass = renderPass;
			fbCI.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbCI.pAttachments = attachments.data();
			fbCI.width = fbProps.width;
			fbCI.height = fbProps.height;
			fbCI.layers = 1;
			VkResult res = vkCreateFramebuffer(device, &fbCI, nullptr, &framebuffers[i]);
			assert(res == VK_SUCCESS);
		}
	}
	void cleanupFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers) {
		for (auto& framebuffer : framebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
	}

	void initBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties& memoryProperties, BufferProperties& props, Buffer& buffer) {
		VkBufferCreateInfo bufferCI{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCI.size = props.size;
		bufferCI.usage = props.bufferUsage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
#ifdef __USE__VMA__
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = props.usage;

		allocationCreateInfo.flags = props.usage == VMA_MEMORY_USAGE_CPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
		VkResult res = vmaCreateBuffer(allocator, &bufferCI, &allocationCreateInfo, &buffer.buffer, &buffer.allocation, &buffer.allocationInfo);
		assert(res == VK_SUCCESS);
		buffer.size = buffer.allocationInfo.size;
#else
		VkResult res = vkCreateBuffer(device, &bufferCI, nullptr, &buffer.buffer);
		assert(res == VK_SUCCESS);
		VkMemoryRequirements memReqs{};
		vkGetBufferMemoryRequirements(device, buffer.buffer, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, memoryProperties, props.memoryProps);
		res = vkAllocateMemory(device, &memAllocInfo, nullptr, &buffer.memory);
		assert(res == VK_SUCCESS);
		res = vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
		assert(res == VK_SUCCESS);

		buffer.size = memReqs.size;
#endif
	}

	void cleanupBuffer(VkDevice device, Buffer& buffer) {
		if (buffer.buffer != VK_NULL_HANDLE) {
#ifdef __USE__VMA__
			vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
#else
			vkFreeMemory(device, buffer.memory, nullptr);
			vkDestroyBuffer(device, buffer.buffer, nullptr);
			buffer.memory = VK_NULL_HANDLE;
			buffer.buffer = VK_NULL_HANDLE;
#endif
		}

	}

	void* mapBuffer(VkDevice device, Buffer& buffer) {
		void* pData{ nullptr };
#ifdef __USE__VMA__
		pData = buffer.allocationInfo.pMappedData;
#else
		VkResult res = vkMapMemory(device, buffer.memory, 0, buffer.size, 0, &pData);
		assert(res == VK_SUCCESS);
#endif
		return pData;
	}

	void unmapBuffer(VkDevice device, Buffer& buffer) {
#ifdef __USE__VMA__
#else

		if (buffer.buffer != VK_NULL_HANDLE) {
			vkUnmapMemory(device, buffer.memory);
		}
#endif
	}

	void flushBuffer(VkDevice device, Buffer& buffer, VkDeviceSize size, VkDeviceSize offset) {
#ifdef __USE__VMA__
		vmaFlushAllocation(allocator, buffer.allocation, offset, size);
#else
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = buffer.memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		VkResult res = vkFlushMappedMemoryRanges(device, 1, &mappedRange);
		assert(res == VK_SUCCESS);
#endif
	}

	void CopyBufferTo(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Buffer& src, Buffer& dst, VkDeviceSize size) {
		VkBufferCopy copyRegion = {};
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
		assert(res == VK_SUCCESS);

		copyRegion.size = size;
		vkCmdCopyBuffer(cmd, src.buffer, dst.buffer, 1, &copyRegion);

		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkFence fence = initFence(device);


		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(res == VK_SUCCESS);

		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(res == VK_SUCCESS);


		vkDestroyFence(device, fence, nullptr);
	}

	void CopyBufferTo(VkDevice device, VkQueue queue, VkCommandBuffer cmd, VkFence fence, Buffer& src, Buffer& dst, VkDeviceSize size) {
		VkBufferCopy copyRegion = {};
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
		assert(res == VK_SUCCESS);

		copyRegion.size = size;
		vkCmdCopyBuffer(cmd, src.buffer, dst.buffer, 1, &copyRegion);

		res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
		assert(res == VK_SUCCESS);

		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(res == VK_SUCCESS);
	}

	VkFence initFence(VkDevice device, VkFenceCreateFlags flags) {
		VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceCI.flags = flags;
		VkFence fence{ VK_NULL_HANDLE };
		VkResult res = vkCreateFence(device, &fenceCI, nullptr, &fence);
		assert(res == VK_SUCCESS);
		return fence;
	}
	void cleanupFence(VkDevice device, VkFence fence) {
		vkDestroyFence(device, fence, nullptr);
	}


	VkDescriptorSetLayout initDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings) {
		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		VkDescriptorSetLayoutCreateInfo descLayoutCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descLayoutCI.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
		descLayoutCI.pBindings = descriptorBindings.data();
		VkResult res = vkCreateDescriptorSetLayout(device, &descLayoutCI, nullptr, &descriptorSetLayout);
		assert(res == VK_SUCCESS);
		return descriptorSetLayout;
	}

	VkDescriptorPool initDescriptorPool(VkDevice device, std::vector<VkDescriptorPoolSize>& descriptorPoolSizes, uint32_t maxSets) {
		VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
		VkDescriptorPoolCreateInfo descPoolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		descPoolCI.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
		descPoolCI.pPoolSizes = descriptorPoolSizes.data();
		descPoolCI.maxSets = maxSets;
		VkResult res = vkCreateDescriptorPool(device, &descPoolCI, nullptr, &descriptorPool);
		assert(res == VK_SUCCESS);
		return descriptorPool;
	}

	VkDescriptorSet initDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		VkDescriptorSetAllocateInfo descAI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		descAI.descriptorPool = descriptorPool;
		descAI.descriptorSetCount = 1;
		descAI.pSetLayouts = &descriptorSetLayout;

		VkResult res = vkAllocateDescriptorSets(device, &descAI, &descriptorSet);
		assert(res == VK_SUCCESS);
		return descriptorSet;
	}

	void initDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkDescriptorSet* pSets, uint32_t setCount) {
		std::vector<VkDescriptorSetLayout> layouts(setCount, descriptorSetLayout);
		VkDescriptorSetAllocateInfo descAI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		descAI.descriptorPool = descriptorPool;
		descAI.descriptorSetCount = setCount;
		descAI.pSetLayouts = layouts.data();

		VkResult res = vkAllocateDescriptorSets(device, &descAI, pSets);
		assert(res == VK_SUCCESS);

	}

	void updateDescriptorSets(VkDevice device, std::vector<VkWriteDescriptorSet> descriptorWrites) {
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void cleanupDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

	void cleanupDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}


	VkPipelineLayout initPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		VkPipelineLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutCI.pSetLayouts = descriptorSetLayout == VK_NULL_HANDLE ? nullptr : &descriptorSetLayout;
		layoutCI.setLayoutCount = descriptorSetLayout == VK_NULL_HANDLE ? 0 : 1;
		VkResult res = vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout);
		assert(res == VK_SUCCESS);
		return pipelineLayout;
	}

	VkPipelineLayout initPipelineLayout(VkDevice device, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) {
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		VkPipelineLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutCI.pSetLayouts = descriptorSetLayouts.data();
		layoutCI.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		VkResult res = vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout);
		assert(res == VK_SUCCESS);
		return pipelineLayout;
	}

	VkPipelineLayout initPipelineLayout(VkDevice device, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, std::vector<VkPushConstantRange>& pushConstants) {
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		VkPipelineLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutCI.pSetLayouts = descriptorSetLayouts.data();
		layoutCI.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		layoutCI.pPushConstantRanges = pushConstants.data();
		layoutCI.pushConstantRangeCount = (uint32_t)pushConstants.size();
		VkResult res = vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout);
		assert(res == VK_SUCCESS);
		return pipelineLayout;
	}

	void cleanupPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
	VkShaderModule initShaderModule(VkDevice device, const std::vector<uint32_t>& spirv) {
		VkShaderModule shaderModule{ VK_NULL_HANDLE };
		VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = spirv.size()*sizeof(uint32_t);
		createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
		VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		assert(res == VK_SUCCESS);
		return shaderModule;
	}
	VkShaderModule initShaderModule(VkDevice device, const char* filename) {
		VkShaderModule shaderModule{ VK_NULL_HANDLE };
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = buffer.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
		VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		assert(res == VK_SUCCESS);
		return shaderModule;
	}


	void cleanupShaderModule(VkDevice device, VkShaderModule shaderModule) {
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}

	VkPipeline initGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout pipelineLayout, std::vector<ShaderModule>& shaders, VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, PipelineInfo& pipelineInfo) {
		VkPipeline pipeline{ VK_NULL_HANDLE };

		//we're working with triangles;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = pipelineInfo.topology;// VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


		// Specify rasterization state. 
		VkPipelineRasterizationStateCreateInfo raster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		//raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.polygonMode = pipelineInfo.polygonMode;
		raster.cullMode = pipelineInfo.cullMode;// VK_CULL_MODE_BACK_BIT;// VK_CULL_MODE_NONE;
		raster.frontFace = pipelineInfo.frontFace;// VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;// 
		raster.lineWidth = 1.0f;
		std::vector<VkPipelineColorBlendAttachmentState> attachments;
		if (pipelineInfo.attachementStates.size() == 0) {
			//all colors, no blending
			VkPipelineColorBlendAttachmentState blendAttachment{};
			blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachment.blendEnable = pipelineInfo.blend;
			//blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			//blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			//blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;// ONE_MINUS_DST_COLOR;
			//blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//blendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
			blendAttachment.srcColorBlendFactor = pipelineInfo.noDraw ? VK_BLEND_FACTOR_ZERO : VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachment.dstColorBlendFactor = pipelineInfo.noDraw ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachments.push_back(blendAttachment);
		}
		else {
			for (auto& att : pipelineInfo.attachementStates) {
				attachments.push_back(att);
			}
		}

		VkPipelineColorBlendStateCreateInfo blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blend.attachmentCount = (uint32_t)attachments.size();
		blend.logicOpEnable = VK_FALSE;
		blend.logicOp = VK_LOGIC_OP_COPY;
		blend.pAttachments = attachments.data();

		//viewport & scissor box

		VkPipelineViewportStateCreateInfo viewportCI{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportCI.viewportCount = 1;
		viewportCI.pViewports = nullptr;
		viewportCI.scissorCount = 1;
		viewportCI.pScissors = nullptr;

		VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		depthStencil.depthTestEnable = pipelineInfo.depthTest ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = pipelineInfo.depthTest || pipelineInfo.stencilTest ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = pipelineInfo.depthCompareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = pipelineInfo.stencilTest ? VK_TRUE : VK_FALSE;
		if (pipelineInfo.stencilTest) {
			depthStencil.front = pipelineInfo.stencil;
			depthStencil.back = pipelineInfo.stencil;
		}

		/*depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;*/

		VkPipelineMultisampleStateCreateInfo multisamplingCI{};
		multisamplingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCI.sampleShadingEnable = VK_FALSE;
		multisamplingCI.rasterizationSamples = pipelineInfo.samples;
		if (pipelineInfo.samples != VK_SAMPLE_COUNT_1_BIT)
			multisamplingCI.minSampleShading = 0.5f; // Optional
		else
			multisamplingCI.minSampleShading = 1.0f; // Optional
		multisamplingCI.pSampleMask = nullptr; // Optional
		multisamplingCI.alphaToCoverageEnable = VK_FALSE; // Optional
		multisamplingCI.alphaToOneEnable = VK_FALSE; // Optional

		VkSpecializationInfo specializationInfo{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (auto& shaderModule : shaders) {
			VkPipelineShaderStageCreateInfo shaderCI{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			shaderCI.stage = shaderModule.stage;
			shaderCI.module = shaderModule.shaderModule;
			shaderCI.pName = "main";
			if (pipelineInfo.specializationData != nullptr && pipelineInfo.specializationSize > 0 && pipelineInfo.specializationMap.size() > 0) {
				if (shaderModule.stage == pipelineInfo.specializationStage) {

					specializationInfo.dataSize = pipelineInfo.specializationSize;
					specializationInfo.mapEntryCount = static_cast<uint32_t>(pipelineInfo.specializationMap.size());
					specializationInfo.pData = pipelineInfo.specializationData;
					specializationInfo.pMapEntries = pipelineInfo.specializationMap.data();
					shaderCI.pSpecializationInfo = &specializationInfo;
				}
			}
			shaderStages.push_back(shaderCI);
		}

		VkPipelineVertexInputStateCreateInfo vertexCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		if (pipelineInfo.noInputState == VK_FALSE) {
			if (!(attributeDescriptions.size() == 0 && bindingDescription.stride == 0)) {

				vertexCI.vertexBindingDescriptionCount = 1;
				vertexCI.pVertexBindingDescriptions = &bindingDescription;
				vertexCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
				vertexCI.pVertexAttributeDescriptions = attributeDescriptions.data();
			}
		}

		//dynamic state (viewport & scissor)
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
		if (pipelineInfo.topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST|| pipelineInfo.topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP) {
			dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
		}
		VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.pDynamicStates = dynamicStates.data();
		dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();


		VkGraphicsPipelineCreateInfo pipe{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipe.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipe.pStages = shaderStages.data();
		pipe.pVertexInputState = &vertexCI;
		pipe.pInputAssemblyState = &inputAssembly;
		pipe.pRasterizationState = &raster;
		pipe.pColorBlendState = &blend;
		pipe.pMultisampleState = &multisamplingCI;
		pipe.pViewportState = &viewportCI;
		pipe.pDepthStencilState = &depthStencil;
		pipe.pDynamicState = &dynamicState;

		pipe.renderPass = renderPass;
		pipe.layout = pipelineLayout;

		VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipe, nullptr, &pipeline);
		assert(res == VK_SUCCESS);

		return pipeline;

	}

	VkPipeline initComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, ShaderModule& shader) {
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineShaderStageCreateInfo shaderCI{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderCI.module = shader.shaderModule;
		shaderCI.pName = "main";
		shaderCI.stage = shader.stage;
		VkComputePipelineCreateInfo computeCI{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		computeCI.stage = shaderCI;
		computeCI.layout = pipelineLayout;
		VkResult res = vkCreateComputePipelines(device, nullptr, 1, &computeCI, nullptr, &pipeline);
		assert(res == VK_SUCCESS);

		return pipeline;
	}


	void cleanupPipeline(VkDevice device, VkPipeline pipeline) {
		vkDestroyPipeline(device, pipeline, nullptr);
	}




	//	void saveImageJPG(VkDevice device, VkCommandBuffer cmd, VkQueue queue, VkImage srcImage, VkImageLayout srcLayout, VkPhysicalDeviceMemoryProperties& memoryProperties, VkFormatProperties& formatProperties, VkFormat colorFormat, uint32_t width, uint32_t height, const char* fileName) {
	//
	//		//cribbed from Sascha Willems code.
	//		bool supportsBlit = (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) && (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);
	//		Image dstImage;
	//		VkImageCreateInfo imageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	//		imageCI.imageType = VK_IMAGE_TYPE_2D;
	//		imageCI.format = colorFormat;
	//		imageCI.extent = { width,height,1 };
	//		imageCI.mipLevels = 1;
	//		imageCI.arrayLayers = 1;
	//		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	//		imageCI.tiling = VK_IMAGE_TILING_LINEAR;
	//		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;// VK_IMAGE_LAYOUT_UNDEFINED;
	//		imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	//		initImage(device, imageCI, dstImage, true);
	//
	//
	//
	//
	//
	//		transitionImage(device, queue, cmd, dstImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	//
	//		transitionImage(device, queue, cmd, srcImage, srcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//
	//		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	//		VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
	//		assert(res == VK_SUCCESS);
	//
	//
	//
	//		if (supportsBlit) {
	//			VkOffset3D blitSize;
	//			blitSize.x = width;
	//			blitSize.y = height;
	//			blitSize.z = 1;
	//
	//			VkImageBlit imageBlitRegion{};
	//			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//			imageBlitRegion.srcSubresource.layerCount = 1;
	//			imageBlitRegion.srcOffsets[1] = blitSize;
	//			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//			imageBlitRegion.dstSubresource.layerCount = 1;
	//			imageBlitRegion.dstOffsets[1];
	//
	//			vkCmdBlitImage(cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//				dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//				1,
	//				&imageBlitRegion,
	//				VK_FILTER_NEAREST);
	//		}
	//		else {
	//			VkImageCopy imageCopyRegion{};
	//
	//			imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//			imageCopyRegion.srcSubresource.layerCount = 1;
	//			imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//			imageCopyRegion.dstSubresource.layerCount = 1;
	//			imageCopyRegion.extent.width = width;
	//			imageCopyRegion.extent.height = height;
	//			imageCopyRegion.extent.depth = 1;
	//
	//			vkCmdCopyImage(cmd,
	//				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//				dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//				1,
	//				&imageCopyRegion);
	//		}
	//
	//		res = vkEndCommandBuffer(cmd);
	//		assert(res == VK_SUCCESS);
	//
	//		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	//		submitInfo.commandBufferCount = 1;
	//		submitInfo.pCommandBuffers = &cmd;
	//
	//		VkFence fence = initFence(device);
	//
	//
	//		res = vkQueueSubmit(queue, 1, &submitInfo, fence);
	//		assert(res == VK_SUCCESS);
	//
	//		res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	//		assert(res == VK_SUCCESS);
	//
	//
	//		vkDestroyFence(device, fence, nullptr);
	//
	//
	//		transitionImage(device, queue, cmd, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	//
	//		transitionImage(device, queue, cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcLayout);
	//
	//		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT,0,0 };
	//		VkSubresourceLayout subResourceLayout;
	//		vkGetImageSubresourceLayout(device, dstImage.image, &subResource, &subResourceLayout);
	//
	//		bool colorSwizzle = false;
	//		if (!supportsBlit)
	//		{
	//			std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
	//			colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), colorFormat) != formatsBGR.end());
	//		}
	//
	//		uint8_t* data{ nullptr };
	//#ifdef __USE__VMA__
	//		data = (uint8_t*)dstImage.allocationInfo.pMappedData;
	//#else
	//		vkMapMemory(device, dstImage.memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	//#endif
	//		data += subResourceLayout.offset;
	//
	//		//std::string filename = std::to_string(index) + ".jpg";
	//		if (colorSwizzle) {
	//			uint32_t* ppixel = (uint32_t*)data;
	//			//must be a better way to do this
	//			for (uint32_t i = 0; i < height; i++) {
	//				for (uint32_t j = 0; j < width; j++) {
	//
	//					uint32_t pix = ppixel[i * width + j];
	//					uint8_t a = (pix & 0xFF000000) >> 24;
	//					uint8_t r = (pix & 0x00FF0000) >> 16;
	//					uint8_t g = (pix & 0x0000FF00) >> 8;
	//					uint8_t b = (pix & 0x000000FF);
	//					uint32_t newPix = (a << 24) | (b << 16) | (g << 8) | r;
	//					ppixel[i * width + j] = newPix;
	//
	//				}
	//			}
	//		}
	//		stbi_write_jpg(fileName, width, height, 4, data, 100);
	//
	//#ifdef __USE__VMA__
	//
	//#else
	//		vkUnmapMemory(device, dstImage.memory);
	//#endif
	//		cleanupImage(device, dstImage);
	//	}


}