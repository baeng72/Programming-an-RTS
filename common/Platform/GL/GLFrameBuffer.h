#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace GL {
	class GLFrameBuffer : public Renderer::FrameBuffer {
		bool _clearonrender;
	public:
		GLFrameBuffer(Renderer::RenderDevice* pdevice,Renderer::Texture** pptextures, uint32_t count, bool clearonrender = true);
		GLFrameBuffer(const GLFrameBuffer& rhs) = delete;
		const GLFrameBuffer& operator=(const GLFrameBuffer& rhs) = delete;
		virtual ~GLFrameBuffer();
		virtual void StartRender() override;
		virtual void EndRender() override;
		virtual void* GetContext()override;
		virtual Renderer::Texture* GetTexture() override;
		virtual void SetClearColor(Renderer::ClearColor* pclrs, uint32_t count) override;
		virtual void DrawVertices(uint32_t count)override;
	};
}