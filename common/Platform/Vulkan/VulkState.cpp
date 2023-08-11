#include "VulkState.h"
#include <GLFW/glfw3.h>

	

namespace Vulkan {



	VulkState::VulkState() {

	}

	VulkState::~VulkState() {

	}

	bool VulkState::Init(VulkStateInitFlags& initFlags, GLFWwindow* windowHandle) {
		std::vector<const char*> requiredExtensions{ "VK_KHR_surface" };
		std::vector<const char*> requiredLayers;

		requiredExtensions.push_back("VK_KHR_win32_surface");


		uint32_t count = 0;
		auto extensions = glfwGetRequiredInstanceExtensions(&count);
		for (uint32_t i = 0; i < count; i++) {
			if (std::find(requiredExtensions.begin(), requiredExtensions.end(), extensions[i]) == requiredExtensions.end())
				requiredExtensions.push_back(extensions[i]);
		}

#if !defined NDEBUG
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		requiredLayers = { "VK_LAYER_KHRONOS_validation" };
		requiredLayers.push_back("VK_LAYER_LUNARG_monitor");
#endif

		instance = initInstance(requiredExtensions, requiredLayers);
#ifdef HZ_PLATFORM_WINDOWS
		requiredExtensions.push_back("VK_KHR_win32_surface");

#endif


		surface = initSurfaceGLFW(instance, windowHandle);


		assert(surface);
		physicalDevice = choosePhysicalDevice(instance, surface, queues);
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

		numSamples = getMaxUsableSampleCount(deviceProperties);
		std::vector<const char*> deviceExtensions;
		if (initFlags.enableSwapchain) {
			deviceExtensions.push_back("VK_KHR_swapchain");
		}
#ifdef ENABLE_DEBUG_MARKER
		if (supportsDeviceExtension(physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		//if (supportsDeviceExtension(mPhysicalDevice, "VK_EXT_debug_utils"))
			//deviceExtensions.push_back("VK_EXT_debug_utils");
#endif


		VkPhysicalDeviceFeatures enabledFeatures{};
		if (deviceFeatures.samplerAnisotropy)
			enabledFeatures.samplerAnisotropy = VK_TRUE;
		if (deviceFeatures.sampleRateShading)
			enabledFeatures.sampleRateShading = VK_TRUE;
		if (initFlags.enableLineWidth && deviceFeatures.wideLines) {
			enabledFeatures.wideLines = VK_TRUE;
		}

		if (initFlags.enableGeometryShader && deviceFeatures.geometryShader)
			enabledFeatures.geometryShader = VK_TRUE;
		if (initFlags.enableWireframe && deviceFeatures.fillModeNonSolid) {
			enabledFeatures.fillModeNonSolid = VK_TRUE;
		}

		uint32_t queueCount = 2;
		device = initDevice(physicalDevice, deviceExtensions, queues, enabledFeatures, queueCount);

#ifdef ENABLE_DEBUG_MARKER
		MARKER_SETUP(device);
#endif
		graphicsQueue = getDeviceQueue(device, queues.graphicsQueueFamily);
		presentQueue = getDeviceQueue(device, queues.presentQueueFamily);
		computeQueue = getDeviceQueue(device, queues.computeQueueFamily);

		backQueue = getDeviceQueue(device, queues.graphicsQueueFamily, 1);

		presentMode = chooseSwapchainPresentMode(presentModes);
		swapchainFormat = chooseSwapchainFormat(surfaceFormats);
		vkGetPhysicalDeviceFormatProperties(physicalDevice, swapchainFormat.format, &formatProperties);

		descriptorSetPoolCache = std::make_unique<DescriptorSetPoolCache>(device);
		descriptorSetLayoutCache = std::make_unique<DescriptorSetLayoutCache>(device);

		commandPool = initCommandPool(device, queues.graphicsQueueFamily);
		commandBuffer = initCommandBuffer(device, commandPool);

		return true;
	}


	void VulkState::Cleanup() {
		vkDeviceWaitIdle(device);

		descriptorSetLayoutCache.reset();
		descriptorSetPoolCache.reset();
		cleanupCommandBuffer(device, commandPool, commandBuffer);
		cleanupCommandPool(device, commandPool);
		cleanupDevice(device);
		cleanupSurface(instance, surface);
		cleanupInstance(instance);
	}
}