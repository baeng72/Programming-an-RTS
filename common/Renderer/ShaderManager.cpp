#include "ShaderManager.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanShaderManager.h"
#include "../Platform/GL/GLShaderManager.h"
namespace Renderer {
	ShaderManager* ShaderManager::Create(Renderer::RenderDevice* pdevice) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLShaderManager(pdevice);
		case Core::API::Vulkan:
			return new Vulkan::VulkanShaderManager(pdevice);
		}
		assert(0);
		return nullptr;
	}
}