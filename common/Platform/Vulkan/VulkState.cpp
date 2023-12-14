#include "VulkState.h"
#include <GLFW/glfw3.h>

	

namespace Vulkan {



	VulkState::VulkState() {

	}

	VulkState::~VulkState() {
		Cleanup();
	}

	bool VulkState::Init(VulkStateInitFlags& initFlags, GLFWwindow* windowHandle) {
		std::vector<const char*> requiredExtensions{ "VK_KHR_surface" };
		std::vector<const char*> requiredLayers;

		uint32_t count = 0;
		auto extensions = glfwGetRequiredInstanceExtensions(&count);
		for (uint32_t i = 0; i < count; i++) {
			if (std::find(requiredExtensions.begin(), requiredExtensions.end(), extensions[i]) == requiredExtensions.end())
				requiredExtensions.push_back(extensions[i]);
		}

#if !defined NDEBUG
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		requiredLayers = { "VK_LAYER_KHRONOS_validation" };
		//requiredLayers.push_back("VK_LAYER_LUNARG_monitor");
#endif

		instance = initInstance(requiredExtensions, requiredLayers);
#ifdef _WIN32
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


		//init default resources
		UniformBufferBuilder::begin(device, deviceProperties, deviceMemoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true)
			.AddBuffer(DEF_UNIFORM_SIZE, 1, 2)//Assume max_frames is 2, doesn't matter
			.build(defResources.defUniform, defResources.defUniformInfo);
		UniformBufferBuilder::begin(device, deviceProperties, deviceMemoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, true)
			.AddBuffer(DEF_UNIFORM_SIZE, 1, 2)//Assume max_frames is 2, doesn't matter
			.build(defResources.defUniformDynamic, defResources.defUniformDynamicInfo);
		UniformBufferBuilder::begin(device, deviceProperties, deviceMemoryProperties, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, true)
			.AddBuffer(DEF_UNIFORM_SIZE, 1, 2)
			.build(defResources.defStorage, defResources.defStorageInfo);
		UniformBufferBuilder::begin(device, deviceProperties, deviceMemoryProperties, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, true)
			.AddBuffer(DEF_UNIFORM_SIZE, 1, 2)
			.build(defResources.defStorageDynamic, defResources.defStorageDynamicInfo);
		constexpr int check_width = 16;
		constexpr int check_height = 16;
		Vulkan::Buffer stagingBuffer = StagingBufferBuilder::begin(device, deviceMemoryProperties)
			.setSize(check_width * check_height)
			.build();
		uint8_t* ppixels = (uint8_t*)Vulkan::mapBuffer(device, stagingBuffer);
		for (int y = 0; y < check_height; y++) {
			for (int x = 0; x < check_width; x++) {
				uint32_t offset = y * check_width + x;
				uint8_t clr = offset & 2 ? 0 : 255;
				ppixels[offset] = clr;
			}
		}
		Vulkan::unmapBuffer(device, stagingBuffer);
		defResources.defTexture = TextureBuilder::begin(device, deviceMemoryProperties)
			.setDimensions(check_width, check_height)
			.setFormat(VK_FORMAT_R8_UNORM)
			.setImageUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)

			.setMipLevels(1)
			.setSampleCount(VK_SAMPLE_COUNT_1_BIT)

			.setFilter(VK_FILTER_LINEAR)
			.build();
		Vulkan::transitionImage(device, backQueue, commandBuffer, defResources.defTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Vulkan::CopyBufferToImage(device, backQueue, commandBuffer, stagingBuffer, defResources.defTexture, check_width, check_height);
		Vulkan::transitionImage(device, backQueue, commandBuffer, defResources.defTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		Vulkan::cleanupBuffer(device, stagingBuffer);

		return true;
	}


	void VulkState::Cleanup() {
		vkDeviceWaitIdle(device);
		Vulkan::cleanupBuffer(device, defResources.defUniform);
		Vulkan::cleanupBuffer(device, defResources.defUniformDynamic);
		Vulkan::cleanupBuffer(device, defResources.defStorage);
		Vulkan::cleanupBuffer(device, defResources.defStorageDynamic);
		Vulkan::cleanupTexture(device, defResources.defTexture);
		descriptorSetLayoutCache.reset();
		descriptorSetPoolCache.reset();
		cleanupCommandBuffer(device, commandPool, commandBuffer);
		cleanupCommandPool(device, commandPool);
		cleanupDevice(device);
		cleanupSurface(instance, surface);
		cleanupInstance(instance);
	}
}