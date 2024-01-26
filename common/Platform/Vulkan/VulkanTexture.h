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
		VkSamplerAddressMode GetSamplerAddrMode(Renderer::TextureSamplerAddress add);
		VkFilter GetSamplerFilter(Renderer::TextureSamplerFilter filter);
	public:
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size, Renderer::TextureSamplerAddress samplerAdd = Renderer::TextureSamplerAddress::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, uint8_t * pixels, Renderer::TextureSamplerAddress samplerAdd = Renderer::TextureSamplerAddress::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerAddress samplerAdd = Renderer::TextureSamplerAddress::Repeat, Renderer::TextureSamplerFilter filter = Renderer::TextureSamplerFilter::Linear);
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, Renderer::Texture* psrc);
		virtual ~VulkanTextureImpl();		
		virtual void* GetNativeHandle()const override;
		virtual glm::vec2 GetScale()const override;
		virtual Renderer::TextureFormat GetFormat()const override { return _fmt; }
		virtual void SetName(const char* pname)override;
		virtual bool SaveToFile(const char* ppath)override;
	};
}

