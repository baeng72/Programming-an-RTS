#include "Texture.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanTexture.h"
#include "../Platform/GL/GLTexture.h"
namespace Renderer {
	Texture* Texture::Create(RenderDevice* pdevice, const char* pfile, TextureSamplerAddress samplerAdd, TextureSamplerFilter filter) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, pfile, glm::vec2(-1.f),samplerAdd,filter);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, pfile, glm::vec2(-1.f), samplerAdd,filter);
		}
		assert(0);
		return nullptr;
	}
	Texture* Texture::Create(RenderDevice* pdevice, const char* pfile, glm::vec2 size, TextureSamplerAddress samplerAdd, TextureSamplerFilter filter) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, pfile, size, samplerAdd,filter);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, pfile, size, samplerAdd,filter);
		}
		assert(0);
		return nullptr;
	}
	Texture* Texture::Create(RenderDevice* pdevice, int width, int height, TextureFormat fmt, uint8_t* pixels, TextureSamplerAddress samplerAdd, TextureSamplerFilter filter) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, width, height, fmt, pixels, samplerAdd,filter);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, width, height, fmt, pixels, samplerAdd,filter);
		}
		assert(0);
		return nullptr;
	}

	Texture* Texture::Create(RenderDevice* pdevice, int width, int height, TextureFormat fmt, TextureSamplerAddress samplerAdd, TextureSamplerFilter filter) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, width, height, fmt, samplerAdd,filter);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, width, height, fmt, samplerAdd,filter);
		}
		assert(0);
		return nullptr;
	}
}