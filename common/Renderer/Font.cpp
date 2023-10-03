#include "Font.h"
#include "../Core/Log.h"
#include "../Core/Api.h"
#include "../Platform/GL/GLFont.h"
#include "../Platform/Vulkan/VulkanFont.h"

namespace Renderer {
	Font* Font::Create() {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLFont();
		case Core::API::Vulkan:
			return new Vulkan::VulkanFont();
		}
		assert(0);
		return nullptr;
	}
}