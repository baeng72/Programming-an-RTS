#include "Texture.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanTexture.h"
#include "../Platform/GL/GLTexture.h"
namespace Renderer {
	Texture* Texture::Create(RenderDevice* pdevice, const char* pfile) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, pfile, glm::vec2(-1.f));
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, pfile, glm::vec2(-1.f));
		}
		assert(0);
		return nullptr;
	}
	Texture* Texture::Create(RenderDevice* pdevice, const char* pfile, glm::vec2 size) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, pfile, size);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, pfile, size);
		}
		assert(0);
		return nullptr;
	}
	Texture* Texture::Create(RenderDevice* pdevice, int width, int height, TextureFormat fmt, uint8_t* pixels) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, width, height, fmt, pixels);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, width, height, fmt, pixels);
		}
		assert(0);
		return nullptr;
	}

	Texture* Texture::Create(RenderDevice* pdevice, int width, int height, TextureFormat fmt) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLTexture(pdevice, width, height, fmt);
		case Core::API::Vulkan:
			return new Vulkan::VulkanTextureImpl(pdevice, width, height, fmt);
		}
		assert(0);
		return nullptr;
	}
}