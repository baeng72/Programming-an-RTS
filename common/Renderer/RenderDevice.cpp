#include "RenderDevice.h"
#include "../Core/Api.h"
#include "../Platform/GL/GLRenderDevice.h"
#include "../Platform/Vulkan/VulkanRenderDevice.h"
namespace Renderer {
	RenderDevice* RenderDevice::Create(void* nativeWindowHandle) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLRenderDevice(nativeWindowHandle);
		case Core::API::Vulkan:
			return new Vulkan::VulkanRenderDevice(nativeWindowHandle);

		}
		assert(0);
		return nullptr;
	}
}