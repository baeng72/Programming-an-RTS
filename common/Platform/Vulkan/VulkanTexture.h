#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Texture.h"
#include "VulkanEx.h"
namespace Vulkan{
	class VulkanTextureImpl : public Renderer::Texture {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Texture _texture;
		glm::vec2 _size;
		Renderer::TextureFormat _fmt;
		Renderer::TextureType _type;
		Renderer::TextureSamplerWrap _addr;
		Renderer::TextureSamplerFilter _filter;
		VkSamplerAddressMode GetSamplerAddrMode(Renderer::TextureSamplerWrap add);
		VkFilter GetSamplerFilter(Renderer::TextureSamplerFilter filter);
		void create(Renderer::RenderDevice* pdevice, Renderer::TextureCreateInfo& ci);
	public:
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size, Renderer::TextureSamplerWrap samplerAdd = Renderer::TextureSamplerWrap::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, uint8_t * pixels, Renderer::TextureSamplerWrap samplerAdd = Renderer::TextureSamplerWrap::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerWrap samplerAdd = Renderer::TextureSamplerWrap::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, Renderer::Texture* psrc);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, Renderer::TextureCreateInfo& ci);
		virtual ~VulkanTextureImpl();		
		virtual void* GetNativeHandle()const override;
		virtual glm::vec2 GetScale()const override;
		virtual void GetDimensions(int& width, int& height)const override { width = _texture.width; height = _texture.height; }
		virtual Renderer::TextureFormat GetFormat()const override { return _fmt; }
		virtual Renderer::TextureFormat GetPixelFormat()const override {
			if (_fmt == Renderer::TextureFormat::FR16G16B16A16)
				return Renderer::TextureFormat::FR32G32B32A32; 
			return _fmt;
		}
		virtual Renderer::TextureType GetType()const override { return _type; }
		virtual bool GetPixels(std::vector<uint8_t>& data, int face)const override;
		virtual void SetName(const char* pname)override;
		virtual bool SaveToFile(const char* ppath)override;
	};
}

