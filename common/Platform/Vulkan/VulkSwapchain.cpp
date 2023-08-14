#include "VulkSwapchain.h"
namespace Vulkan {
	VulkSwapchain::VulkSwapchain() {
		_ASSERTE(_CrtCheckMemory());
#ifdef __USE__VECTOR__
		size_t framesize = framebuffers.size();
#else
		for (int i = 0; i < MAX_FRAME_COUNT; i++) {
			framebuffers[i] = VK_NULL_HANDLE;
			swapchainImages[i] = VK_NULL_HANDLE;
			swapchainImageViews[i] = VK_NULL_HANDLE;
			presentCompletes[i] = VK_NULL_HANDLE;
			renderCompletes[i] = VK_NULL_HANDLE;
			fences[i] = VK_NULL_HANDLE;
			commandPools[i] = VK_NULL_HANDLE;
			commandBuffers[i] = VK_NULL_HANDLE;
		}
#endif
		_ASSERTE(_CrtCheckMemory());
	}

	VulkSwapchain::~VulkSwapchain() {
		Destroy();
		if (swapchain != VK_NULL_HANDLE)
			cleanupSwapchain(device, swapchain);
	}

	void VulkSwapchain::Create(VkDevice device, VkSurfaceKHR surface, VkQueue graphicsQueue, VkQueue presentQueue, VkPhysicalDeviceMemoryProperties memoryProperties, VulkSwapchainFlags& flags) {
#ifdef __USE__VECTOR__
		size_t framesize = framebuffers.size();
#else
#endif
		_ASSERTE(_CrtCheckMemory());
		this->device = device;
		this->surface = surface;
		this->graphicsQueue = graphicsQueue;
		this->presentQueue = presentQueue;
		this->memoryProperties = memoryProperties;
		this->flags = flags;
		this->maxFrames = flags.imageCount;

		if (flags.clearValueCount > 0 && flags.clearValues) {
#ifdef __USE__VECTOR__
			clearValues.resize(flags.clearValueCount);
#else
			assert(flags.clearValueCount <= 3);
#endif
			for (uint32_t i = 0; i < flags.clearValueCount; i++) {
				clearValues[i] = flags.clearValues[i];
			}
		}
		else {
#ifdef __USE__VECTOR__
			clearValues.resize(2);
#endif
			clearValues[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues[1] = { 0.f,1.f };
		}
#ifdef __USE__VECTOR__
		if (presentCompletes.size()) {
			for (auto presentComplete : presentCompletes) {
				cleanupSemaphore(device, presentComplete);
			}
		}
		if (renderCompletes.size()) {
			for (auto renderComplete : renderCompletes) {
				cleanupSemaphore(device, renderComplete);
			}
		}
		if (fences.size()) {
			for (auto fence : fences) {
				cleanupFence(device, fence);
			}
		}
		presentCompletes.resize(flags.imageCount);
		renderCompletes.resize(flags.imageCount);
#else
		for (int i = 0; i < MAX_FRAME_COUNT; i++) {
			if (presentCompletes[i] != VK_NULL_HANDLE)
				cleanupSemaphore(device, presentCompletes[i]);
			if (renderCompletes[i] != VK_NULL_HANDLE)
				cleanupSemaphore(device, renderCompletes[i]);
			if (fences[i] != VK_NULL_HANDLE)
				cleanupFence(device, fences[i]);
		}
#endif
		for (uint32_t i = 0; i < flags.imageCount; i++) {

			VkSemaphore presentComplete = initSemaphore(device);

			presentCompletes[i] = presentComplete;
			VkSemaphore renderComplete = initSemaphore(device);

			renderCompletes[i] = renderComplete;
			VkFence fence = initFence(device, VK_FENCE_CREATE_SIGNALED_BIT);
#ifdef __USE__VECTOR__
			fences.push_back(fence);
#else
			fences[i] = fence;
#endif
#ifdef ENABLE_DEBUG_MARKER
			char buffer[128];
			sprintf_s(buffer, "PresentComplete%d", i);
			NAME_SEMAPHORE(presentComplete, buffer);
			sprintf_s(buffer, "RenderComplete%d", i);
			NAME_SEMAPHORE(renderComplete, buffer);
			sprintf_s(buffer, "Fence%d", i);
			NAME_FENCE(fence, buffer);
#endif
		}
#ifdef __USE__VECTOR__
		initCommandPools(device, flags.imageCount, flags.graphicsQueueFamily, commandPools);
		initCommandBuffers(device, commandPools, commandBuffers);
#else
		initCommandPools(device, flags.imageCount, flags.graphicsQueueFamily, commandPools);
		initCommandBuffers(device, commandPools, commandBuffers, flags.imageCount);
#endif

		VkSwapchainKHR old = swapchain;
		VkExtent2D extent = { flags.clientWidth,flags.clientHeight };
		swapchain = initSwapchain(device, surface, flags.clientWidth, flags.clientHeight, flags.surfaceCaps, flags.presentMode, flags.format, extent, flags.imageCount, old);

		assert(swapchain);
		if (old != VK_NULL_HANDLE)
			cleanupSwapchain(device, old);
#ifdef __USE__VECTOR__
		getSwapchainImages(device, swapchain, swapchainImages);
		flags.imageCount = (uint32_t)swapchainImages.size();
		initSwapchainImageViews(device, swapchainImages, flags.format.format, swapchainImageViews);
#else
		getSwapchainImages(device, swapchain, swapchainImages, flags.imageCount);
		imageCount = flags.imageCount;
		initSwapchainImageViews(device, swapchainImages, flags.format.format, swapchainImageViews, imageCount);
#endif


		ImageProperties props;
		props.width = flags.clientWidth;
		props.height = flags.clientHeight;
		props.samples = flags.samples;
#ifdef __USE__VMA__
		props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
		props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif

		if (flags.enableMSAA) {

			props.format = flags.format.format;
			props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
			initImage(device, memoryProperties, props, msaaImage);
		}
		if (flags.enableDepthBuffer) {
			props.format = flags.depthFormat;
			props.samples = VK_SAMPLE_COUNT_1_BIT;
			props.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | flags.depthImageUsage;
			props.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			initImage(device, memoryProperties, props, depthImage);

			//initDepthImage(mDevice, mDepthFormat, mFormatProperties, mMemoryProperties, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mMSAA ? mNumSamples : VK_SAMPLE_COUNT_1_BIT, mClientWidth, mClientHeight, mDepthImage);
		}
		props.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		//setup render pass based on config
		RenderPassProperties rpProps;
		rpProps.colorFormat = flags.format.format;
		rpProps.sampleCount = flags.enableMSAA ? flags.samples : VK_SAMPLE_COUNT_1_BIT;
		rpProps.depthFormat = flags.enableDepthBuffer ? flags.depthFormat : VK_FORMAT_UNDEFINED;
		rpProps.resolveFormat = flags.enableMSAA ? rpProps.colorFormat : VK_FORMAT_UNDEFINED;
		renderPass = initRenderPass(device, rpProps);
		FramebufferProperties fbProps;
		fbProps.colorAttachments = swapchainImageViews;
		fbProps.colorAttachmentCount = imageCount;
		fbProps.depthAttachment = flags.enableDepthBuffer ? depthImage.imageView : VK_NULL_HANDLE;
		fbProps.resolveAttachment = flags.enableMSAA ? msaaImage.imageView : VK_NULL_HANDLE;
		fbProps.width = flags.clientWidth;
		fbProps.height = flags.clientHeight;
#ifdef __USE__VECTOR__
		initFramebuffers(device, renderPass, fbProps, framebuffers);
#else
		initFramebuffers(device, renderPass, fbProps, framebuffers);
#endif
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea = { 0,0,(uint32_t)flags.clientWidth,(uint32_t)flags.clientHeight };
#ifdef __USE__VECTOR__
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
#else
		for (uint32_t i = 0; i < flags.clearValueCount; i++) {
			clearValues[i] = flags.clearValues[i];
		}
		clearValueCount = flags.clearValueCount;
		renderPassBeginInfo.clearValueCount = (uint32_t)flags.clearValueCount;

		renderPassBeginInfo.pClearValues = clearValues;
#endif
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.commandBufferCount = 1;
		presentInfo.swapchainCount = 1;
		presentInfo.pImageIndices = &index;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.waitSemaphoreCount = 1;
		currFrame = UINT32_MAX;
	}

	void VulkSwapchain::Destroy() {
		vkDeviceWaitIdle(device);
#ifdef __USE__VECTOR__
		cleanupCommandBuffers(device, commandPools, commandBuffers);
		commandBuffers.clear();
		cleanupCommandPools(device, commandPools);
		commandPools.clear();
		if (presentCompletes.size()) {
			for (auto presentComplete : presentCompletes) {
				cleanupSemaphore(device, presentComplete);
			}
		}
		presentCompletes.clear();
		if (renderCompletes.size()) {
			for (auto renderComplete : renderCompletes) {
				cleanupSemaphore(device, renderComplete);
			}
		}
		renderCompletes.clear();
		if (fences.size()) {
			for (auto fence : fences) {
				cleanupFence(device, fence);
			}
		}
		fences.clear();
		cleanupFramebuffers(device, framebuffers);
		framebuffers.clear();
#else
		for (int i = 0; i < MAX_FRAME_COUNT; i++) {
			if (presentCompletes[i] != VK_NULL_HANDLE) {
				cleanupSemaphore(device, presentCompletes[i]);
				presentCompletes[i] = VK_NULL_HANDLE;
			}
			if (renderCompletes[i] != VK_NULL_HANDLE) {
				cleanupSemaphore(device, renderCompletes[i]);
				renderCompletes[i] = VK_NULL_HANDLE;
			}
			if (fences[i] != VK_NULL_HANDLE) {
				cleanupFence(device, fences[i]);
				fences[i] = VK_NULL_HANDLE;
			}
		}
#endif
		if (msaaImage.image != VK_NULL_HANDLE) {
			cleanupImage(device, msaaImage);
			msaaImage.image = VK_NULL_HANDLE;
		}
		if (depthImage.image != VK_NULL_HANDLE) {
			cleanupImage(device, depthImage);
			depthImage.image = VK_NULL_HANDLE;
		}
		if (renderPass != VK_NULL_HANDLE) {
			cleanupRenderPass(device, renderPass);
			renderPass = VK_NULL_HANDLE;
		}
#ifdef __USE_VECTOR__
		if (swapchainImageViews.size()) {
			cleanupSwapchainImageViews(device, swapchainImageViews);
			swapchainImageViews.clear();
		}
#else
		cleanupSwapchainImageViews(device, swapchainImageViews, imageCount);
#endif
	}

	uint32_t VulkSwapchain::NextFrame(uint64_t timeout) {
		currFrame = (currFrame + 1) % maxFrames;
		currFence = fences[currFrame];
		vkWaitForFences(device, 1, &currFence, VK_TRUE, timeout);
		vkResetFences(device, 1, &currFence);
		return currFrame;
	}

	void VulkSwapchain::Resize(uint32_t width, uint32_t height) {
		flags.clientWidth = width;
		flags.clientHeight = height;
#ifdef __USE__VECTOR__
		flags.clearValues = clearValues.data();
		flags.clearValueCount = (uint32_t)clearValues.size();
#else
		flags.clearValueCount = (uint32_t)clearValueCount;
		flags.clearValues = clearValues;
#endif
		Destroy();
		Create(device, surface, graphicsQueue, presentQueue, memoryProperties, flags);
	}

	VkCommandBuffer VulkSwapchain::BeginRender(bool startRenderPass) {



		VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentCompletes[currFrame], nullptr, &index);
		assert(res == VK_SUCCESS);

		VkCommandBuffer cmd = commandBuffers[index];

		vkBeginCommandBuffer(cmd, &beginInfo);

		renderPassBeginInfo.framebuffer = framebuffers[index];
		if (startRenderPass)
			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		return cmd;
	}

	void VulkSwapchain::StartRenderPass(VkCommandBuffer cmd) {
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkSwapchain::EndRender(VkCommandBuffer cmd, bool present) {
		vkCmdEndRenderPass(cmd);

		VkResult res = vkEndCommandBuffer(cmd);
		assert(res == VK_SUCCESS);

		submitInfo.pWaitSemaphores = &presentCompletes[currFrame];
		submitInfo.pSignalSemaphores = &renderCompletes[currFrame];
		submitInfo.pCommandBuffers = &cmd;
		res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, currFence);
		assert(res == VK_SUCCESS);
		if (present) {


			presentInfo.pWaitSemaphores = &renderCompletes[currFrame];
			res = vkQueuePresentKHR(presentQueue, &presentInfo);
			assert(res == VK_SUCCESS);
		}
		frameCount++;

	}
}