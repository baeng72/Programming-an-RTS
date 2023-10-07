#include "Line2D.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanLine2D.h"
#include "../Platform/GL/GLLine2D.h"
namespace Renderer {
	
	Line2D* Line2D::Create(RenderDevice* pdevice) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLLine2D(pdevice);
		case Core::API::Vulkan:
			return new Vulkan::VulkanLine2D(pdevice);
		}
		assert(0);
		return nullptr;
	}
	
}