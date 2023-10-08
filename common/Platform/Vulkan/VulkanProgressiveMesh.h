#pragma once

#include "../../mesh/ProgressiveMesh.h"
#include "../vulkan/VulkanEx.h"
namespace Vulkan {

	class VulkanProgressiveMesh : public Mesh::ProgressiveMesh {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Buffer _vertexBuffer;
		Vulkan::Buffer _indexBuffer;
		VkDeviceSize _vertSize;
		VkDeviceSize _indSize;
		
		VkDeviceSize _vertexStride;
		uint32_t _rawVertexCount;
		uint32_t _rawIndexCount;
		uint32_t _currIndexCount;
		uint32_t _currOffset;
		uint32_t* _pIndexBuffer;
		//keep track of original data
		std::vector<float> _rawVertices;
		std::vector<uint32_t> _rawIndices;
		
	public:
		VulkanProgressiveMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize,uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes);
		virtual ~VulkanProgressiveMesh();
		virtual void SetIndexCount(uint32_t indexCount)override;
		virtual uint32_t GetIndexCount()override;
		virtual void Render()override;
	};

}