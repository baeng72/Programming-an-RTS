#include "MultiMesh.h"
#include "../Core/Api.h"
#include "../Platform/Vulkan/VulkanMultiMesh.h"
#include "../Platform/GL/GLMultiMesh.h"
namespace Mesh {
	
	MultiMesh* MultiMesh::Create(Renderer::RenderDevice* pdevice, mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices,  std::vector<Mesh::Primitive>& primitives,Renderer::VertexAttributes&vertexAttributes) {
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLMultiMesh(pdevice, xform, vertices, indices, primitives,vertexAttributes);
		case Core::API::Vulkan:
			return new Vulkan::VulkanMultiMesh(pdevice, xform, vertices, indices, primitives,vertexAttributes);
		}
		assert(0);
		return nullptr;
	}
	
}