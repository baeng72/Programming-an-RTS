#pragma once
#include "RenderDevice.h"
//#include <glm/glm.hpp>
namespace Renderer {
	enum class TextureFormat{R8,R8G8,R8G8B8,R8G8B8A8,D32,FR32G32B32,FR32G32B32A32,FR16G16B16A16};
	enum class TextureSamplerWrap{Repeat,Clamp,Border};
	enum class TextureSamplerFilter {Linear,Nearest};
	enum class TextureType {Tex2D,TexCube};
	enum class TextureFunction{Sampler=0x01,Target=0x02,Source=0x04,Destination=0x08};
	struct TextureCreateInfo {
		int width{ 0 };
		int height{ 0 };
		TextureFormat format{ TextureFormat::R8G8B8A8 };
		uint8_t* pixels{ nullptr };
		TextureSamplerWrap wrap{ TextureSamplerWrap::Repeat };
		TextureSamplerFilter filter{ TextureSamplerFilter::Linear };
		TextureType type{ TextureType::Tex2D };
		TextureFunction function{ TextureFunction::Sampler };
		bool mipMaps{ false };
	};
	class Texture {
	public:
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile,TextureSamplerWrap samplerAdd=TextureSamplerWrap::Repeat,TextureSamplerFilter filter=TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size, TextureSamplerWrap samplerAdd = TextureSamplerWrap::Repeat, TextureSamplerFilter filter = TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, int width, int height, TextureFormat fmt, uint8_t* pixels, TextureSamplerWrap samplerAdd = TextureSamplerWrap::Clamp, TextureSamplerFilter filter = TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, int width, int height, TextureFormat fmt, TextureSamplerWrap samplerAdd = TextureSamplerWrap::Repeat, TextureSamplerFilter filter = TextureSamplerFilter::Linear);
		static Texture* Create(Renderer::RenderDevice* pdevice, Renderer::Texture* psrc);
		static Texture* Create(Renderer::RenderDevice* pdevice, TextureCreateInfo& ci);
		virtual ~Texture() = default;		
		virtual void* GetNativeHandle()const = 0;
		virtual glm::vec2 GetScale()const = 0;
		virtual void GetDimensions(int&width, int&height)const = 0;
		virtual TextureFormat GetFormat()const = 0;
		virtual TextureType GetType()const = 0;
		virtual TextureFormat GetPixelFormat()const = 0;
		virtual bool GetPixels(std::vector<uint8_t>& data, int face)const = 0;
		virtual void SetName(const char* pname) = 0;
		virtual bool SaveToFile(const char* ppath) = 0;
	};
}