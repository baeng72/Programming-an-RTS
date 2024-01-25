#pragma once
#include "RenderDevice.h"
//#include <glm/glm.hpp>
namespace Renderer {
	enum class TextureFormat{R8,R8G8,R8G8B8,R8G8B8A8,D32};
	enum class TextureSamplerAddress{Repeat,Clamp,Border};
	enum class TextureSamplerFilter {Linear,Nearest};
	class Texture {
	public:
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile,TextureSamplerAddress samplerAdd=TextureSamplerAddress::Clamp,TextureSamplerFilter filter=TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size, TextureSamplerAddress samplerAdd = TextureSamplerAddress::Clamp, TextureSamplerFilter filter = TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, int width, int height, TextureFormat fmt, uint8_t* pixels, TextureSamplerAddress samplerAdd = TextureSamplerAddress::Clamp, TextureSamplerFilter filter = TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, int width, int height, TextureFormat fmt, TextureSamplerAddress samplerAdd = TextureSamplerAddress::Clamp, TextureSamplerFilter filter = TextureSamplerFilter::Linear);
		virtual ~Texture() = default;		
		virtual void* GetNativeHandle()const = 0;
		virtual glm::vec2 GetScale()const = 0;
		virtual TextureFormat GetFormat()const = 0;
		virtual void SetName(const char* pname) = 0;
		virtual bool SaveToFile(const char* ppath) = 0;
	};
}