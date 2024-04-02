#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Texture.h"
#include <glad/glad.h>
namespace GL {
	struct GLTextureInfo {
		GLuint textureID;
		int width;
		int height;
		GLenum wrapMode;
		GLenum filter;
		GLenum format;
		GLenum type;
	};
	class GLTexture : public Renderer::Texture {
		Renderer::RenderDevice* _pdevice;
		GLuint _textureID;
		vec2 _size;
		int _width;
		int _height;
		int _channels;
		Renderer::TextureFormat _fmt;
		Renderer::TextureType _type;
		GLTextureInfo tex;
		GLenum GetSamplerAddressMode(Renderer::TextureSamplerWrap addrMode);
		GLenum GetSamplerFilter(Renderer::TextureSamplerFilter filter);
		void create(Renderer::RenderDevice* pdevice, Renderer::TextureCreateInfo& ci);
	public:		
		GLTexture(Renderer::RenderDevice* pdevice, const char* pfile, glm::vec2 size, Renderer::TextureSamplerWrap wrap = Renderer::TextureSamplerWrap::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, uint8_t* pixels, Renderer::TextureSamplerWrap wrap = Renderer::TextureSamplerWrap::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerWrap wrap = Renderer::TextureSamplerWrap::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		GLTexture(Renderer::RenderDevice* pdevice, Renderer::Texture* psrc);
		GLTexture(Renderer::RenderDevice* pdevice, Renderer::TextureCreateInfo& ci);
		virtual ~GLTexture();
		virtual void* GetNativeHandle()const override;
		virtual glm::vec2 GetScale()const override;
		virtual void GetDimensions(int& width, int& height)const override { width = _width; height = _height; }
		virtual Renderer::TextureFormat GetFormat()const override { return _fmt; }
		virtual Renderer::TextureFormat GetPixelFormat()const override {
			if (_fmt == Renderer::TextureFormat::FR16G16B16A16)
				return Renderer::TextureFormat::FR32G32B32A32; 
			return _fmt;
		}
		virtual Renderer::TextureType GetType()const override { return _type; }
		virtual bool GetPixels(std::vector<uint8_t>& data, int face)const override;
		virtual void SetName(const char*pname)override {};
		virtual bool SaveToFile(const char* ppath)override;
	};
}