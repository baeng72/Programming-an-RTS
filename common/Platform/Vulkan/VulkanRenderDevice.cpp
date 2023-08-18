#pragma once

#include "VulkanRenderDevice.h"
#include "VulkanShaderManager.h"
#include "../../Core/Log.h"
Renderer::RenderDevice* Renderer::RenderDevice::Create(void* nativeWindowHandle) {
	return new Vulkan::VulkanRenderDevice(nativeWindowHandle);
}
namespace Vulkan {

	

	VulkanRenderDevice::VulkanRenderDevice(void* nativeWindowHandle) :_enableVSync(true),_enableGeometry(false),_enableDepthBuffer(false)
	{
		_window = reinterpret_cast<GLFWwindow*>(nativeWindowHandle);
		


	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		vkDeviceWaitIdle(_context.device);
		_swapchain.reset();
		_state.reset();
	}

	void VulkanRenderDevice::Init()
	{
		_state = std::make_unique<VulkState>();
		VulkStateInitFlags initFlags;
		initFlags.enableGeometryShader = _enableGeometry;
		initFlags.enableWireframe = _enableLines;
		_state->Init(initFlags, _window);
		auto physicalDevice = _state->getPhysicalDevice();
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		_context = _state->getContext();
		LOG_TRACE("Device: {0}", properties.deviceName);

		_swapchain = std::make_unique<VulkSwapchain>();
		VulkSwapchainFlags flags;
		flags.imageCount = 2;//double-buffer
		flags.presentMode = _enableVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
		
		VkClearValue clearValues[2] = {
			{0.f,0.f,0.f,1.f} ,
			{1.f,0.f}
		};
		flags.clearValueCount = _enableDepthBuffer ? 2 : 1;
		flags.clearValues = clearValues;
		flags.enableDepthBuffer = _enableDepthBuffer;
		
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);
		flags.clientWidth = width;
		flags.clientHeight = height;
		flags.surfaceCaps = _state->getSurfaceCapabilities();
		_swapchain->Create(_context.device, _state->getSurface(), _state->getGraphicsQueue(), _state->getPresentQueue(), _context.memoryProperties, flags);
		_frameData.renderPass = _swapchain->getRenderPass();
	}

	void VulkanRenderDevice::StartRender()
	{
		_swapchain->NextFrame();
		_cmd = _swapchain->BeginRender();
		_frameData.cmd = _cmd;
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);
		VkViewport viewport = { 0,0,(float)width,(float)height,0.f,1.f };
		vkCmdSetViewport(_cmd, 0, 1, &viewport);
		VkRect2D scissor{ 0,0,(uint32_t)width,(uint32_t)height };
		vkCmdSetScissor(_cmd, 0, 1, &scissor);
	}

	void VulkanRenderDevice::EndRender()
	{
		_swapchain->EndRender(_cmd);

		_cmd = VK_NULL_HANDLE;
		_frameData.cmd = _cmd;
	}

	void VulkanRenderDevice::SetVSync(bool vsync)
	{
		_enableVSync = vsync;//only useful before Init() is called
	}

	void VulkanRenderDevice::EnableGeometry(bool geom) {
		_enableGeometry = geom;
	}
	void VulkanRenderDevice::EnableLines(bool lines) {
		_enableLines = lines;
	}
	void VulkanRenderDevice::EnableDepthBuffer(bool depth) {
		_enableDepthBuffer = depth;
	}

	void* VulkanRenderDevice::GetDeviceContext() const
	{
		return (void*)&_context;
	}

	void* VulkanRenderDevice::GetCurrentFrameData()const {
		return (void*)&_frameData;
	}

	void VulkanRenderDevice::Wait() const
	{
		vkDeviceWaitIdle(_context.device);
	}


	void VulkanRenderDevice::GetDimensions(int* width, int* height)const {
		glfwGetFramebufferSize(_window, width, height);
	}
	void VulkanRenderDevice::SetClearColor(float r, float g, float b, float a)
	{
		_swapchain->SetClearColor(0, { r,g,b,a });
	}
}