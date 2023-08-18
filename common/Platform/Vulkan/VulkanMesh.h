#pragma once
#include <glm/glm.hpp>
#include "../../Renderer/Mesh.h"
#include "../../Renderer/Shader.h"
#include "VulkanEx.h"
namespace Renderer {
	class VulkanMesh : public Renderer::Mesh {
		RenderDevice* _pdevice;
		Vulkan::Buffer _vertexBuffer;
		Vulkan::Buffer _indexBuffer;
		uint32_t		_indexCount;
	public:
		VulkanMesh(RenderDevice* pdevice,float*pvertices,uint32_t vertSize, uint32_t*pindices,uint32_t indSize);
		virtual ~VulkanMesh();
		void Render(Renderer::Shader*pshader)override;
	};
}
