#include "GLFrameBuffer.h"

namespace GL {
	GLFrameBuffer::GLFrameBuffer(Renderer::RenderDevice* pdevice, Renderer::Texture**pptextures,uint32_t count,bool clearonrender)
		:_clearonrender(clearonrender)
	{
		_pdevice = pdevice;
	}
	GLFrameBuffer::~GLFrameBuffer()
	{
	}
	void GLFrameBuffer::StartRender()
	{
	}
	void GLFrameBuffer::EndRender()
	{
	}
	void* GLFrameBuffer::GetContext()
	{
		return nullptr;
	}
	Renderer::Texture* GLFrameBuffer::GetTexture()
	{
		return nullptr;
	}
	void GLFrameBuffer::SetClearColor(Renderer::ClearColor* pclrs, uint32_t count)
	{
	}
	void GLFrameBuffer::DrawVertices(uint32_t count)
	{
	}
}