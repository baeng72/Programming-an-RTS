#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace GL {
	class GLFrameBuffer : public Renderer::FrameBuffer {
		bool _clearonrender;
		std::vector<GLuint> _fbos;
		std::vector<Renderer::Texture*> _textures;//frame buffer texture	
		Renderer::Texture *_depthMap;
		std::vector<GLuint> _textureHandles;
		GLuint _depthHandle;
		uint32_t _currFrame;
		uint32_t _frameCount;
		Color _clearColors[2];
		uint32_t _clearColorCount;
		GLint _width;
		GLint _height;
		GLint _viewport[4];
	public:
		GLFrameBuffer(Renderer::RenderDevice* pdevice,Renderer::Texture** pptextures, uint32_t count,Renderer::Texture*pdepthmap, bool clearonrender = true,bool clonedevice=false);
		GLFrameBuffer(const GLFrameBuffer& rhs) = delete;
		const GLFrameBuffer& operator=(const GLFrameBuffer& rhs) = delete;
		virtual ~GLFrameBuffer();
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