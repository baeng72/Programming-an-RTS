#include "ProgressiveMesh.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanProgressiveMesh.h"
#include "../Platform/GL/GLProgressiveMesh.h"
namespace Mesh {
	ProgressiveMesh* ProgressiveMesh::Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize,  uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes) {
		switch (Core::GetAPI()) {
		case Core::API::Vulkan:
			return new Vulkan::VulkanProgressiveMesh(pdevice, pvertices, vertSize,  pindices, indSize,vertexAttributes);
		case Core::API::GL:
			return new GL::GLProgressiveMesh(pdevice, pvertices, vertSize,  pindices, indSize, vertexAttributes);

		}
		assert(0);
		return nullptr;
		
	}
}