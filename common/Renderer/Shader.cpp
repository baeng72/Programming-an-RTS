#include "Shader.h"
#include "../Platform/Vulkan/VulkanShader.h"
#include "../Platform/GL/GLShader.h"
#include "../Core/Api.h"
namespace Renderer {
	Shader* Shader::Create(RenderDevice* pdevice, void* shaderData) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLShader(pdevice, shaderData);
		case Core::API::Vulkan:
			return new Vulkan::VulkanShader(pdevice, shaderData);
		}
		assert(0);
		return nullptr;
	}
}