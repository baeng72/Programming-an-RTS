#include "Sprite.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanSprite.h"
#include "../Platform/GL/GLSprite.h"
namespace Renderer{
	Sprite* Sprite::Create(Renderer::RenderDevice* pdevice) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLSprite(pdevice);
		case Core::API::Vulkan:
			return new Vulkan::VulkanSprite(pdevice);
		}
		assert(0);
		return nullptr;
		
	}
}