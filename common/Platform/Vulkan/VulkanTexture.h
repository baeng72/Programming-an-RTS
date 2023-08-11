#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Texture.h"
#include "VulkanEx.h"
namespace Vulkan{
	class VulkanTextureImpl : public Renderer::Texture {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Texture _texture;
		glm::vec2 _size;
	public:
		VulkanTextureImpl(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size);
		virtual ~VulkanTextureImpl();		
		virtual void* GetNativeHandle()const override;
		virtual glm::vec2 GetScale()const override;
	};
}

