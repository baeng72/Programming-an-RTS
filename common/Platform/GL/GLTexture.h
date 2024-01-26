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
			GLenum addrMode;
			GLenum filter;
			GLenum format;
		}tex;
		GLenum GetSamplerAddressMode(Renderer::TextureSamplerAddress addrMode);
		GLenum GetSamplerFilter(Renderer::TextureSamplerFilter filter);
	public:		
		GLTexture(Renderer::RenderDevice* pdevice, const char* pfile, glm::vec2 size, Renderer::TextureSamplerAddress samplerAdd = Renderer::TextureSamplerAddress::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, uint8_t* pixels, Renderer::TextureSamplerAddress samplerAdd = Renderer::TextureSamplerAddress::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerAddress samplerAdd = Renderer::TextureSamplerAddress::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		GLTexture(Renderer::RenderDevice* pdevice, Renderer::Texture* psrc);
		virtual ~GLTexture();
		virtual void* GetNativeHandle()const override;
		virtual glm::vec2 GetScale()const override;
		virtual Renderer::TextureFormat GetFormat()const override { return _fmt; }
		virtual void SetName(const char*pname)override {};
		virtual bool SaveToFile(const char* ppath)override;
	};
}