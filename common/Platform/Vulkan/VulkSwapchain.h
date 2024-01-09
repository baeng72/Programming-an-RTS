#pragma once
#include "../../Core/profiler.h"
#ifdef __USE__VECTOR__
#include <vector>
#else
#define MAX_FRAME_COUNT 2
#endif
#include "VulkanEx.h"
#define DEFAULT_SCREEN_WIDTH 1280
#define DEFAULT_SCREEN_HEIGHT 720
namespace Vulkan {
	struct VulkFrameData {
		VkCommandBuffer cmd;
		VkRenderPass	renderPass;
	};
	struct VulkSwapchainFlags {
		uint32_t							clientWidth;
		uint32_t							clientHeight;
		VkSurfaceCapabilitiesKHR			surfaceCaps;
		VkPresentModeKHR					presentMode;
		VkSurfaceFormatKHR					format;
		uint32_t							imageCount;
		uint32_t							graphicsQueueFamily;
		VkSampleCountFlagBits				samples;
		bool								enableDepthBuffer;
		VkFormat							depthFormat;
		VkImageUsageFlags					depthImageUsage;
		bool								enableMSAA;
		VkClearValue* clearValues;
		uint32_t							clearValueCount;
		VulkSwapchainFlags() {
			clientWidth = DEFAULT_SCREEN_HEIGHT;
			clientHeight = DEFAULT_SCREEN_HEIGHT;
			memset(&surfaceCaps, 0, sizeof(VkSurfaceCapabilitiesKHR));
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			format.format = PREFERRED_IMAGE_FORMAT;
			format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			imageCount = 2;
			graphicsQueueFamily = 0;
			samples = VK_SAMPLE_COUNT_1_BIT;
			enableDepthBuffer = true;
			depthFormat = VK_FORMAT_D32_SFLOAT;
			depthImageUsage = 0;
			enableMSAA = false;
			clearValues = nullptr;
			clearValueCount = 0;
		}
	};
	class VulkSwapchain {
		VulkSwapchainFlags					flags;
		VkDevice							device{ VK_NULL_HANDLE };
		VkSurfaceKHR						surface{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties	memoryProperties{};
		VkSwapchainKHR						swapchain{ VK_NULL_HANDLE };
		VkQueue								graphicsQueue{ VK_NULL_HANDLE };
		VkQueue								presentQueue{ VK_NULL_HANDLE };
		VkPresentModeKHR					presentMode{ VK_PRESENT_MODE_MAILBOX_KHR };
		VkSurfaceFormatKHR					swapchainFormat;
		VkFormatProperties					formatProperties{};
		VkExtent2D							swapchainExtent;
#ifdef __USE__VECTOR__
		std::vector<VkImage>                swapchainImages;
		std::vector<VkImageView>            swapchainImageViews;
		std::vector<VkSemaphore>            presentCompletes;
		std::vector<VkSemaphore>            renderCompletes;
		std::vector<VkFence>                fences;
		std::vector<VkCommandPool>          commandPools;
		std::vector<VkCommandBuffer>		commandBuffers;
		std::vector<VkFramebuffer>          framebuffers;
		std::vector<VkClearValue>			clearValues;
#else
		VkImage								swapchainImages[MAX_FRAME_COUNT];
		VkImageView							swapchainImageViews[MAX_FRAME_COUNT];
		VkSemaphore							presentCompletes[MAX_FRAME_COUNT];
		VkSemaphore							renderCompletes[MAX_FRAME_COUNT];
		VkFence								fences[MAX_FRAME_COUNT];
		VkCommandPool						commandPools[MAX_FRAME_COUNT];
		VkCommandBuffer						commandBuffers[MAX_FRAME_COUNT];
		VkFramebuffer						framebuffers[MAX_FRAME_COUNT];
		uint32_t							imageCount{ 0 };
		VkClearValue						clearValues[3];
		uint32_t							clearValueCount{ 0 };
#endif

		VkFence								currFence{ VK_NULL_HANDLE };

		VkRenderPass                        renderPass{ VK_NULL_HANDLE };
		Image                       depthImage;
		Image                       msaaImage;

		VkCommandBufferBeginInfo            beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		VkRenderPassBeginInfo               renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		VkPipelineStageFlags                submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo		                submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		VkPresentInfoKHR                    presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		uint32_t		                    index = 0;
		
		uint32_t		                            frameCount = 0;
		uint32_t		                            currFrame{ (uint32_t)(-1) };
		uint32_t		                            maxFrames{ 0 };

	public:
		VulkSwapchain();
		void operator=(VulkSwapchain const&) = delete;
		~VulkSwapchain();
		void Create(VkDevice device, VkSurfaceKHR surface, VkQueue graphicsQueue, VkQueue presentQueue, VkPhysicalDeviceMemoryProperties memoryProperties, VulkSwapchainFlags& flags);
		void Destroy();
		operator VkDevice()const { return device; }
		operator VkSwapchainKHR()const { return swapchain; }
		VkDevice getDevice()const { return device; }
		VkSwapchainKHR getSwapchain()const { return swapchain; }
		VkCommandBuffer getCurrentCommandBuffer()const { return commandBuffers[index]; }
		uint32_t	NextFrame(uint64_t timeout = UINT64_MAX);
		VkCommandBuffer BeginRender(bool startRenderPass = true);
		void EndRender(VkCommandBuffer cmd, bool present = true);
		void StartRenderPass(VkCommandBuffer);
		void Resize(uint32_t width, uint32_t height);
		VkRenderPass getRenderPass()const { return renderPass; }
		uint32_t getFrameCount()const { return frameCount; }
		uint32_t getCurrFrame()const { return currFrame; }
		void SetClearColor(int index, VkClearColorValue value) { clearValues[index].color = value; }
	};
}