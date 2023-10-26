#include "VIBuffer.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanVIBuffer.h"
#include "../Platform/GL/GLVIBuffer.h"

namespace Renderer {
	
	VIBuffer* VIBuffer::Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& attributes, bool optimize)
	{
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLVIBuffer(pdevice, pvertices, vertSize, pindices, indSize, attributes, optimize);
		case Core::API::Vulkan:
			return new Vulkan::VulkanVIBufferImpl(pdevice, pvertices, vertSize, pindices, indSize, attributes, optimize);
		}
		assert(0);
		return nullptr;
	}
}