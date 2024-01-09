#include "GLFrameBuffer.h"
#include "GLERR.h"
namespace GL {
	GLFrameBuffer::GLFrameBuffer(Renderer::RenderDevice* pdevice, Renderer::Texture**pptextures,uint32_t count,Renderer::Texture*pdepthmap,bool clearonrender,bool clonedevice)
		:_clearonrender(clearonrender), _currFrame(0),_depthHandle(0)
	{
		_pdevice = pdevice;
		_textures.insert(_textures.end(), pptextures, &pptextures[count]);
		_textureHandles.resize(count);
		_frameCount = count;
		_fbos.resize(count);
		glGenFramebuffers(count, _fbos.data());
		for (uint32_t i = 0; i < count; i++) {
			_width = _height = 0;
			GLuint handle = (*(GLuint*)pptextures[i]->GetNativeHandle());
			glGetTextureLevelParameteriv(handle,0, GL_TEXTURE_WIDTH,&_width);
			glGetTextureLevelParameteriv(handle,0, GL_TEXTURE_HEIGHT, &_height);
			GLERR();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, handle);
			glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D,0);
			glBindFramebuffer(GL_FRAMEBUFFER, _fbos[i]);
			glNamedFramebufferTexture(_fbos[i], GL_COLOR_ATTACHMENT0, handle, 0);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			_textureHandles[i] = handle;
			GLERR();
		}
		if (pdepthmap) {
			_depthMap = pdepthmap;
			GLuint handle = *(GLuint*)pdepthmap->GetNativeHandle();
			for (uint32_t i = 0; i < count; i++) {
				glNamedFramebufferTexture(_fbos[i], GL_DEPTH_ATTACHMENT, handle, 0);
				if (count == 0) {
					//probably doing shadow mapping
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);
				}
				GLERR();
			}
			_depthHandle = handle;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLERR();

		_clearColors[0] = Color( 0.f,0.f,0.f,1.f );
		_clearColorCount = 1u;
		
	}
	GLFrameBuffer::~GLFrameBuffer()
	{
		glDeleteBuffers(2, _fbos.data());
	}
	void GLFrameBuffer::StartRender()
	{
		_currFrame++;
		_currFrame %= _frameCount;
		glBindFramebuffer(GL_FRAMEBUFFER, _fbos[_currFrame]);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureHandles[_currFrame], 0); 		
		glGetIntegerv(GL_VIEWPORT, _viewport);
		glViewport(0, 0, _width, _height);
		if (_clearonrender) {
			glClearColor(_clearColors[0].x, _clearColors[0].y, _clearColors[0].z, _clearColors[0].w);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		GLERR();
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}
	void GLFrameBuffer::EndRender()
	{		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, _viewport[2], _viewport[3]);
		GLERR();
	}
	void* GLFrameBuffer::GetContext()
	{
		return nullptr;
	}
	void* GLFrameBuffer::GetNativeHandle()
	{
		return &_fbos[_currFrame];
	}
	Renderer::Texture* GLFrameBuffer::GetTexture()
	{
		int ind = _currFrame + 1;
		ind %= _frameCount;
		return _textures[ind];
	}
	void GLFrameBuffer::SetClearColor(Renderer::ClearColor* pclrs, uint32_t count)
	{
		assert(count < 3);
		for (uint32_t i = 0; i < count; i++) {
			_clearColors[i] = pclrs->color;
		}
		_clearColorCount = count;
	}
	void GLFrameBuffer::DrawVertices(uint32_t count)
	{
		glDrawArrays(GL_TRIANGLES, 0, 6);
		GLERR();
	}

	void GLFrameBuffer::Clear(Rect& r, Color clr) {
		
		
		glEnable(GL_SCISSOR_TEST);
		GLERR();
		glScissor(r.left, _height - r.bottom, r.Width(), r.Height());//flip y
		GLERR();
		glClearColor(clr.r, clr.g, clr.b, clr.a);
		GLenum clearFlags = GL_COLOR_BUFFER_BIT;
		/*if (_depthTest)
			clearFlags |= GL_DEPTH_BUFFER_BIT;*/
		glClear(clearFlags);
		GLERR();
		glDisable(GL_SCISSOR_TEST);
		GLERR();
	}
}