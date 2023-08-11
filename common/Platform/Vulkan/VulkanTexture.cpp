#pragma once
#include "VulkanTexture.h"
#include "VulkState.h"
#include <Windows.h>
Renderer::Texture* Renderer::Texture::Create(Renderer::RenderDevice* pdevice, const char* pfile) {
	return new Vulkan::VulkanTextureImpl(pdevice,pfile,glm::vec2(-1.f));
}
Renderer::Texture* Renderer::Texture::Create(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size) {
	return new Vulkan::VulkanTextureImpl(pdevice, pfile,size);
}
namespace Vulkan {
	VulkanTextureImpl::VulkanTextureImpl(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size):_size(size)
	{
		
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		std::vector<Vulkan::Texture> textures;
		char buffer[512];
		GetCurrentDirectoryA(sizeof(buffer), buffer);
		TextureLoader::begin(context.device, context.commandBuffer, context.queue, context.memoryProperties)
			.addTexture(pfile)
			.load(textures);
		_texture = textures[0];
		if (_size.x == -1 || _size.y == -1) {
			_size = glm::vec2(_texture.width, _texture.height);
		}
	}
	VulkanTextureImpl::~VulkanTextureImpl()
	{
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		vkDeviceWaitIdle(context.device);
		Vulkan::cleanupTexture(context.device, _texture);
	}
	
	void* VulkanTextureImpl::GetNativeHandle() const
	{
		return (void*)&_texture;
	}

	glm::vec2 VulkanTextureImpl::GetScale()const {
		glm::vec2 scale = glm::vec2(_size.x/(float)_texture.width,_size.y/(float) _texture.height);
		return scale;
	}
}
