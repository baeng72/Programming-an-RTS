#pragma once
#include <glm/glm.hpp>
#include "../../Mesh/Mesh.h"
#include "../../Renderer/Shader.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanMesh : public Mesh::Mesh {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Buffer _vertexBuffer;
		Vulkan::Buffer _indexBuffer;
		uint32_t		_indexCount;
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize);
	public:
		VulkanMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize);
		VulkanMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertStride, uint32_t vertCount, uint32_t* pindices, uint32_t indSize, bool optimize = true);		
		virtual ~VulkanMesh();
		void Render(Renderer::Shader*pshader)override;
	};
}
