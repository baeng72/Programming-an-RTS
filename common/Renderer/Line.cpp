#include "Line.h"
#include "../Core/Api.h"
#include "../Platform/GL/GLLine.h"
#include "../Platform/Vulkan/VulkanLine.h"
namespace Renderer {
	Line* Line::Create(RenderDevice* pdevice, LineVertex* pvertices, uint32_t vertexCount, float lineWidth, bool isLineList) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLLine(pdevice, pvertices, vertexCount, lineWidth, isLineList);
		case Core::API::Vulkan:
			return new Vulkan::VulkanLine(pdevice, pvertices, vertexCount, lineWidth, isLineList);
		}
		assert(0);
		return nullptr;
	}
}