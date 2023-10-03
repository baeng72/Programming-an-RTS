#include "Mesh.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanMesh.h"
#include "../Platform/GL/GLMesh.h"

namespace Mesh {

	Mesh* Mesh::Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize,Renderer::VertexAttributes&vertexAttributes,bool optimize) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLMesh(pdevice, pvertices, vertSize, pindices, indSize,vertexAttributes,optimize);
		case Core::API::Vulkan:
			return new Vulkan::VulkanMesh(pdevice, pvertices, vertSize, pindices, indSize,vertexAttributes,optimize);
		}
		assert(0);
		return nullptr;
	}
	/*Mesh* Mesh::Create(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertStride, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, bool optimize) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLMesh(pdevice, pvertices, vertStride, vertSize, pindices, indSize, optimize);
		case Core::API::Vulkan:
			return new Vulkan::VulkanMesh(pdevice, pvertices, vertStride, vertSize, pindices, indSize, optimize);
		}
		assert(0);
		return nullptr;
	}*/
}