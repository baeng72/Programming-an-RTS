#include "VulkanProgressiveMesh.h"
#include <meshoptimizer/src/meshoptimizer.h>
#include "../Vulkan/VulkState.h"
#include "../Vulkan/VulkSwapchain.h"
#include "../Vulkan/VulkanEx.h"



namespace Vulkan {




	VulkanProgressiveMesh::VulkanProgressiveMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes)
		:_pdevice(pdevice)
	{
		uint32_t vertStride = vertexAttributes.vertexStride;
		uint32_t indCount = indSize / sizeof(uint32_t);
		uint32_t vertCount = vertSize / vertStride;
		std::vector<uint32_t> indices(pindices, pindices + indCount);

		std::vector<unsigned int> remap(indCount); // allocate temporary memory for the remap table
		size_t vertex_count = meshopt_generateVertexRemap(remap.data(), pindices, indCount, pvertices, vertCount, vertStride);
		_rawIndices.resize(indCount);
		meshopt_remapIndexBuffer(_rawIndices.data(), pindices, indCount, remap.data());
		_rawVertices.resize(vertCount * vertStride);
		meshopt_remapVertexBuffer(_rawVertices.data(), pvertices, vertCount, vertStride, remap.data());
		
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		{


			std::vector<uint32_t> vertexLocations;
			Vulkan::VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddVertices(vertSize, _rawVertices.data())
				
				.build(_vertexBuffer, vertexLocations);
			_rawVertexCount = vertSize / vertStride;
			_vertexStride = vertStride;
			_vertSize = vertSize;
		}
		{
			std::vector<uint32_t> indexLocations;
			Vulkan::IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddIndices(indSize, _rawIndices.data(),true)				
				.AddIndices(indSize, _rawIndices.data(), true)
				.build(_indexBuffer, indexLocations,(void**)&_pIndexBuffer);
			
			_indSize = indSize;
			_rawIndexCount = _currIndexCount = indSize / sizeof(uint32_t);
			_currOffset = 0;
		}
	}

	VulkanProgressiveMesh::~VulkanProgressiveMesh()
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;

		Vulkan::cleanupBuffer(context.device, _vertexBuffer);
		Vulkan::cleanupBuffer(context.device, _indexBuffer);
	}

	void VulkanProgressiveMesh::SetIndexCount(uint32_t indexCount)
	{
		if (indexCount == _rawIndexCount)
			return;
		if (indexCount > _rawIndexCount) {
			indexCount = _rawIndexCount;
		}
			
		float resultError = 0;		
		float targetError = 1e-2f;
		std::vector<uint32_t> lodIndices(_rawIndexCount);
		lodIndices.resize(meshopt_simplify(lodIndices.data(), _rawIndices.data(), _rawIndexCount, _rawVertices.data(), _rawVertexCount, _vertexStride, indexCount, 0.1f, 0, &resultError));
		uint32_t offset = _currOffset;
		offset++;
		offset %= 2;
		uint32_t lodIndexCount = (uint32_t)lodIndices.size();
		uint32_t lodIndexSize = lodIndexCount * sizeof(uint32_t);
		memcpy(&_pIndexBuffer[offset * _rawIndexCount], lodIndices.data(), lodIndexSize);
		//TODO: Multi-threading---this could be atomic if multi-threading
		_currIndexCount = lodIndexCount;
		_currOffset = offset;
	}

	uint32_t VulkanProgressiveMesh::GetIndexCount()
	{
		return _currIndexCount;
	}

	void VulkanProgressiveMesh::Render()
	{
		
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;
		vkCmdBindIndexBuffer(frameData.cmd, _indexBuffer.buffer, _currOffset*_indSize, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(frameData.cmd, 0, 1, &_vertexBuffer.buffer, offsets);
		vkCmdDrawIndexed(frameData.cmd, _currIndexCount, 1, 0, 0, 0);
	}

}