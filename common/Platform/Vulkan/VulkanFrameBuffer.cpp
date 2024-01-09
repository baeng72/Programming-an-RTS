#include "VulkanFrameBuffer.h"
#include "VulkanRenderDevice.h"
#include "VulkState.h"
#include "VulkSwapchain.h"

#include "VulkanDebug.h"
#include "../../Core/Log.h"
namespace Vulkan {
	VulkanFrameBuffer::VulkanFrameBuffer(Renderer::RenderDevice* pdevice, Renderer::Texture** ptextures, uint32_t count, Renderer::Texture* pdepthmap, bool clearonrender,bool clonedevice)
		:_width(0), _height(0), _format(VK_FORMAT_UNDEFINED), _frameCount(0), _clearonrender(clearonrender), _currFrame(0),_depthMap(pdepthmap),_clonedevice(clonedevice)
	{
		ASSERT(count > 0, "Count must be greater than 0!");
		_frameCount=count;
		_pdevice = pdevice;
		_textures.insert(_textures.end(), ptextures, &ptextures[count]);
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		std::vector<VkImageView> imageViews;
		VkImageView depthImageView = VK_NULL_HANDLE;
		_depthFormat = VK_FORMAT_UNDEFINED;
		
		if (pdepthmap) {
			Texture* pnative = (Texture*)pdepthmap->GetNativeHandle();
			depthImageView = pnative->imageView;
			_depthFormat = pnative->format;
		}
		
		
		for (uint32_t i = 0; i < count; i++) {
			Texture* pnative = (Texture*)ptextures[i]->GetNativeHandle();
			_format = pnative->format;
			_width = pnative->width;
			_height = pnative->height;
			imageViews.push_back(pnative->imageView);
			if (!clearonrender) {
				transitionImage(context.device, context.queue,context.commandBuffer, pnative->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
		LOG_INFO("Creating Framebuffer {0},{1}", _width, _height);
		if (clonedevice) {
			_renderPass = RenderPassBuilder::begin(context.device)
				.setColorFormat(_format)
				.setDepthFormat(_depthFormat)
				.setColorInitialLayout(clearonrender ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.setColorFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.setColorLoadOp(clearonrender ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD)
				.setColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
				.setDepthInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.setDepthFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.setDepthStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
				.build();
			
		}
		else {
			_renderPass = RenderPassBuilder::begin(context.device)
				.setColorFormat(_format)
				.setDepthFormat(_depthFormat)
				.setColorInitialLayout(clearonrender ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.setColorFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.setColorLoadOp(clearonrender ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD)
				.setColorStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
				.setDepthInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.setDepthFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.setDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT)
				.setDependency(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT)
				.build();
		}
		

		
		
		FramebufferBuilder::begin(context.device)
			.setDimensions(_width, _height)
			.setColorImageViews(imageViews)
			.setDepthImageView(depthImageView)
			.setRenderPass(_renderPass)
			.build(_framebuffers);
		
			
		
		_clearValues[0] = { { 0.f,0.f,0.f,1.f } };
		_clearValueCount = 1u;
		if (_depthFormat != VK_FORMAT_UNDEFINED) {
			_clearValues[1] = { {1.f,0.f} };
			_clearValueCount = 2u;
		}
	}
	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		
		cleanupFramebuffers(context.device, _framebuffers);		
		cleanupRenderPass(context.device, _renderPass);

	}
	void VulkanFrameBuffer::StartRender()
	{
		_currFrame++;
		_currFrame %= _frameCount;		
		Texture*tex = (Texture*)_textures[_currFrame]->GetNativeHandle();
		
		auto name = getImageName(*tex);
		//LOG_INFO("Using attached texture: {0}", name);
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		VkCommandBuffer cmd = framedata.cmd;
		VkViewport viewport = { 0.0f,0.0f,(float)_width,(float)_height,0.0f,1.0f };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		VkRect2D scissor = { {0,0},{(uint32_t)_width,(uint32_t)_height} };
		vkCmdSetScissor(cmd, 0, 1, &scissor);
		VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		
		beginInfo.pClearValues = _clearValues;
		beginInfo.clearValueCount = _clearValueCount;

		beginInfo.renderPass = _renderPass;
		beginInfo.framebuffer = _framebuffers[_currFrame];
		beginInfo.renderArea = { 0,0,(uint32_t)_width,(uint32_t)_height };
		vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void VulkanFrameBuffer::EndRender()
	{
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		VkCommandBuffer cmd = framedata.cmd;
		vkCmdEndRenderPass(cmd);
	}
	void* VulkanFrameBuffer::GetContext()
	{
		return &_renderPass;
	}
	Renderer::Texture* VulkanFrameBuffer::GetTexture()
	{
		int ind = _currFrame + 1;
		ind %= _frameCount;
		return _textures[ind];
	}
	void* VulkanFrameBuffer::GetNativeHandle()
	{
		return &_framebuffers[_currFrame];
	}
	void VulkanFrameBuffer::SetClearColor(Renderer::ClearColor* pclrs, uint32_t count)
	{
		for (uint32_t i = 0; i < std::min(2u, count); i++) {
			_clearValues[i] = {  pclrs[i].color.r,pclrs[i].color.g,pclrs[i].color.b,pclrs[i].color.a  };
		}
	}
	void VulkanFrameBuffer::DrawVertices(uint32_t count)
	{
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		VkCommandBuffer cmd = framedata.cmd;
		vkCmdDraw(cmd,count, 1, 0, 0);
	}

	void VulkanFrameBuffer::Clear(Rect& r, Color clr) {
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		VkCommandBuffer cmd = framedata.cmd;
		VkClearAttachment attachments[1] = { 0 };
		attachments[0].colorAttachment = 0;
		attachments[0].clearValue.color = VkClearColorValue{ clr.r,clr.g,clr.b,clr.a };
		attachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkClearRect rects[1] = { 0 };
		rects[0].rect = { { (int32_t)r.left,(int32_t)r.top},{(uint32_t)(r.right - r.left),(uint32_t)(r.bottom - r.top)} };
		rects[0].baseArrayLayer = 0;
		rects[0].layerCount = 1;
		vkCmdClearAttachments(framedata.cmd, 1, attachments, 1, rects);
	}
}