

#include "VulkanRenderDevice.h"
#include "VulkanShaderManager.h"
#include "../../Core/Log.h"

namespace Vulkan {

	

	VulkanRenderDevice::VulkanRenderDevice(void* nativeWindowHandle) :_enableVSync(true),_enableGeometry(false),_enableDepthBuffer(false)
	{
		_window = reinterpret_cast<GLFWwindow*>(nativeWindowHandle);
		


	}

	VulkanRenderDevice::~VulkanRenderDevice()
	{
		vkDeviceWaitIdle(_context.device);
		cleanupRenderPass(_context.device,_shadowData.renderPass);
		std::vector<VkFramebuffer> framebuffers = { _shadowData.frameBuffer };
		cleanupFramebuffers(_context.device,framebuffers);
		cleanupTexture(_context.device, _shadowData.shadowMap);
		_swapchain.reset();
		_state.reset();
	}

	void VulkanRenderDevice::Init()
	{
		_state = std::make_unique<VulkState>();
		VulkStateInitFlags initFlags;
		initFlags.enableGeometryShader = _enableGeometry;
		initFlags.enableWireframe = _enableWireframe;
		initFlags.enableLineWidth = _enableLines;
		_state->Init(initFlags, _window);
		auto physicalDevice = _state->getPhysicalDevice();
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		_context = _state->getContext();
		LOG_TRACE("Device: {0}", properties.deviceName);

		_swapchain = std::make_unique<VulkSwapchain>();
		VulkSwapchainFlags flags;
		flags.imageCount = 2;//double-buffer
		flags.presentMode = _enableVSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
		
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

		_shadowData.shadowMap = TextureBuilder::begin(_context.device, _context.memoryProperties)
			.setDimensions(width, height)
			.setFormat(VK_FORMAT_D32_SFLOAT)
			.setImageAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
			.setImageUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			.build();

		_shadowData.renderPass = RenderPassBuilder::begin(_context.device)
			.setDepthFormat(VK_FORMAT_D32_SFLOAT)
			.setDepthStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
			.setDepthInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.setDepthFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.build();
		std::vector<VkFramebuffer> framebuffers;
		FramebufferBuilder::begin(_context.device)
			.setDimensions(width, height)
			.setDepthImageView(_shadowData.shadowMap.imageView)
			.setRenderPass(_shadowData.renderPass)
			.build(framebuffers);
		_shadowData.frameBuffer = framebuffers[0];
	}

	void VulkanRenderDevice::StartRender()
	{
		//EASY_FUNCTION(profiler::colors::Cyan);
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
		//EASY_FUNCTION(profiler::colors::Red);
		_swapchain->EndRender(_cmd);

		_cmd = VK_NULL_HANDLE;
		_frameData.cmd = _cmd;
	}

	void VulkanRenderDevice::StartShadowRender()
	{
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);
	}

	void VulkanRenderDevice::EndShadowRender()
	{
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
	void VulkanRenderDevice::EnableWireframe(bool wireframe) {
		_enableWireframe = wireframe;
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
	void VulkanRenderDevice::Clear(Rect& r, Color clr)
	{
		VkClearAttachment attachments[1] = { 0 };
		attachments[0].colorAttachment = 0;
		attachments[0].clearValue.color = VkClearColorValue{ clr.r,clr.g,clr.b,clr.a };
		attachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkClearRect rects[1] = { 0 };
		rects[0].rect = { { (int32_t)r.left,(int32_t)r.top},{(uint32_t)(r.right - r.left),(uint32_t)(r.bottom - r.top)} };
		rects[0].baseArrayLayer = 0;
		rects[0].layerCount = 1;
		vkCmdClearAttachments(_cmd, 1, attachments, 1, rects);
	}

	void VulkanRenderDevice::SetViewport(ViewPort& vp)
	{
		VkViewport viewport = { vp.x,vp.y,vp.width,vp.height,vp.fnear,vp.ffar };
		vkCmdSetViewport(_cmd, 0, 1, &viewport);
	}
	
}