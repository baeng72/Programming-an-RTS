#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/FrameBuffer.h"

#include "VulkanEx.h"

namespace Vulkan {

	class VulkanFrameBuffer : public Renderer::FrameBuffer {		
		std::vector<VkFramebuffer> _framebuffers;	//might need 2 frame buffers
		VkRenderPass _renderPass;			//the render pass
		bool _clonedevice;
		std::vector<Renderer::Texture*> _textures;//frame buffer texture	
		Renderer::Texture* _depthMap;
		uint32_t _currFrame;
		uint32_t _frameCount;
		uint32_t _width;
		uint32_t _height;
		VkFormat _format;
		VkFormat _depthFormat;
		VkClearValue _clearValues[2];
		uint32_t _clearValueCount;
		bool _clearonrender;
	public:
		VulkanFrameBuffer(Renderer::RenderDevice* pdevice, Renderer::Texture** pptextures, uint32_t count,Renderer::Texture*pdepthmap=nullptr, bool clearonrender = true, bool clonedevice = false);
		VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;
		const VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;
		virtual ~VulkanFrameBuffer();
		virtual void StartRender() override;
		virtual void EndRender() override;
		virtual void* GetContext()override;
		virtual void* GetNativeHandle()override;
		virtual Renderer::Texture* GetTexture() override;
		virtual void SetClearColor(Renderer::ClearColor* pclrs, uint32_t count) override;
		virtual void DrawVertices(uint32_t count)override;
		virtual void Clear(Rect& r, Color clr) override;
	};
}