#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Texture.h"
#include <glad/glad.h>
namespace GL {
	class GLTexture : public Renderer::Texture {
		Renderer::RenderDevice* _pdevice;
		GLuint _textureID;
		vec2 _size;
		int _width;
		int _height;
		int _channels;
		Renderer::TextureFormat _fmt;
		struct GLTextureInfo {
			int textureID;
			int width;
			int height;
		}tex;
	public:		
		GLTexture(Renderer::RenderDevice* pdevice, const char* pfile, glm::vec2 size);
		GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, uint8_t* pixels);
		GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt );
		virtual ~GLTexture();
		virtual void* GetNativeHandle()const override;
		virtual glm::vec2 GetScale()const override;
		virtual Renderer::TextureFormat GetFormat()const override { return _fmt; }
		virtual void SetName(const char*pname)override {};
		virtual bool SaveToFile(const char* ppath)override;
	};
}