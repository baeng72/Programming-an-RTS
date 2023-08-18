#pragma once
#include <GLFW/glfw3.h>
#include <memory>
#include "VulkanEx.h"
namespace Vulkan {
	struct VulkContext {
		VkDevice device;
		VkQueue	queue;
		VkCommandBuffer commandBuffer;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceMemoryProperties memoryProperties;

		DescriptorSetPoolCache* pPoolCache;
		DescriptorSetLayoutCache* pLayoutCache;

	};

	struct VulkStateInitFlags {
		bool        enableGeometryShader{ false };

		bool		enableWireframe{ false };
#ifdef _DEBUG
		bool		enableValidation{ true };
#else
		bool		enableValidation{ false };
#endif
		bool		enableLineWidth{ false };
		bool		enableSwapchain{ true };
	};

	class VulkState {
		VulkStateInitFlags					initFlags;
		VkInstance							instance{ VK_NULL_HANDLE };
		VkSurfaceKHR						surface{ VK_NULL_HANDLE };
		VkPhysicalDevice					physicalDevice{ VK_NULL_HANDLE };
		Queues						queues;
		VkQueue								graphicsQueue{ VK_NULL_HANDLE };
		VkQueue								presentQueue{ VK_NULL_HANDLE };
		VkQueue								computeQueue{ VK_NULL_HANDLE };
		VkQueue								backQueue{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties			deviceProperties;
		VkPhysicalDeviceMemoryProperties	deviceMemoryProperties;
		VkPhysicalDeviceFeatures			deviceFeatures;
		VkSurfaceCapabilitiesKHR			surfaceCaps{};
		std::vector<VkSurfaceFormatKHR>		surfaceFormats;
		std::vector<VkPresentModeKHR>		presentModes;
		VkSampleCountFlagBits				numSamples{ VK_SAMPLE_COUNT_1_BIT };
		VkDevice							device{ VK_NULL_HANDLE };
		VkPresentModeKHR					presentMode{  };
		VkSurfaceFormatKHR					swapchainFormat{};
		VkFormatProperties					formatProperties{};
		VkCommandPool						commandPool{ VK_NULL_HANDLE };
		VkCommandBuffer						commandBuffer{ VK_NULL_HANDLE };

		/*VkExtent2D							swapchainExtent{};
		VkSwapchainKHR						swapchain{ VK_NULL_HANDLE };
		std::vector<VkImage>				swapchainImages;
		std::vector<VkImageView>			swapchainImageViews;
		std::vector<VkSemaphore>			presentCompletes;
		std::vector<VkSemaphore>			renderCompletes;
		std::vector<VkFence>				fences;
		VkFence								currFence{ VK_NULL_HANDLE };
		VkCommandPool						commandPool{ VK_NULL_HANDLE };
		VkCommandBuffer						commandBuffer{ VK_NULL_HANDLE };
		VkCommandBuffer						currCommandBuffer{ VK_NULL_HANDLE };
		std::vector<VkCommandPool>			commandPools;
		std::vector<VkCommandBuffer>		commandBuffers;
		VkFormat							depthFormat{ VK_FORMAT_D32_SFLOAT };
		VkImageUsageFlags					depthImageUsage{ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT };
		VkRenderPass						renderPass{ VK_NULL_HANDLE };
		Image						depthImage;
		Image						msaaImage;
		std::vector<VkFramebuffer>			framebuffers;*/
		std::unique_ptr<DescriptorSetLayoutCache> descriptorSetLayoutCache;
		std::unique_ptr<DescriptorSetPoolCache> descriptorSetPoolCache;
	public:
		VulkState();
		void operator=(VulkState const&) = delete;
		~VulkState();
		bool Init(VulkStateInitFlags& initFlags, GLFWwindow* mainWindow);
		void Cleanup();
		void Resize(uint32_t width, uint32_t height);



		VkInstance							getInstance()const { return instance; }
		VkSurfaceKHR						getSurface()const { return surface; }
		VkPhysicalDevice					getPhysicalDevice()const { return physicalDevice; }

		uint32_t									getGraphicsQueueFamily()const { return queues.graphicsQueueFamily; }
		uint32_t									getPresentQueueFamily()const { return queues.presentQueueFamily; }
		uint32_t									getComputeQueueFamily()const { return queues.computeQueueFamily; }
		VkQueue								getGraphicsQueue()const { return graphicsQueue; }
		VkQueue								getPresentQueue()const { return presentQueue; }
		VkQueue								getComputeQueue()const { return computeQueue; }
		VkPhysicalDeviceProperties			getPhysicalDeviceProperties()const { return deviceProperties; }
		VkPhysicalDeviceMemoryProperties	getPhysicalDeviceMemoryProperties()const { return deviceMemoryProperties; }
		VkPhysicalDeviceFeatures			getPhysicalDeviceFeatures()const { return deviceFeatures; }
		VkSurfaceCapabilitiesKHR			getSurfaceCapabilities()const { return surfaceCaps; }
		VkSampleCountFlagBits				getSampleCount()const { return numSamples; }
		VkDevice							getDevice()const { return device; }
		VulkContext							getContext()const { return { device,backQueue,commandBuffer,deviceProperties,deviceMemoryProperties,descriptorSetPoolCache.get(),descriptorSetLayoutCache.get() }; }
		/*VkPresentModeKHR					getPresentMode()const { return presentMode; }
		VkSurfaceFormatKHR					getSwapchainFormat()const { return swapchainFormat; }
		VkFormatProperties					getFormatProperties()const { return formatProperties; }
		VkExtent2D							getSwapchainExtent()const { return swapchainExtent; }*/


	};

	//extern VulkState vulkState;
}