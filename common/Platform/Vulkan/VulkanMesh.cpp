
#include "../../Core/defines.h"
#include "../../Core/hash.h"
#include "VulkanMesh.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "../meshoptimizer/src/meshoptimizer.h"

namespace Vulkan {
	VulkanMesh::VulkanMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& attributes,bool optimize) :_pdevice(pdevice) {
		
		if (optimize) {
			uint32_t indCount = indSize / sizeof(uint32_t);
			uint32_t vertCount = vertSize / attributes.vertexStride;
			std::vector<uint32_t> indices(pindices, pindices + indCount);

			std::vector<unsigned int> remap(indCount); // allocate temporary memory for the remap table
			size_t vertex_count = meshopt_generateVertexRemap(remap.data(), pindices, indCount, pvertices, vertCount, attributes.vertexStride);
			std::vector<uint32_t> progressiveIndices(indCount);
			meshopt_remapIndexBuffer(progressiveIndices.data(), pindices, indCount, remap.data());
			std::vector<float> progressiveVertices(vertCount * attributes.vertexStride);
			meshopt_remapVertexBuffer(progressiveVertices.data(), pvertices, vertCount, attributes.vertexStride, remap.data());
			Create(progressiveVertices.data(), vertSize, progressiveIndices.data(), indSize);
		}
		else {
			Create(pvertices, vertSize, pindices, indSize);
		}
	}
	
	
	VulkanMesh::~VulkanMesh()
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		
		Vulkan::cleanupBuffer(context.device, _vertexBuffer);
		Vulkan::cleanupBuffer(context.device, _indexBuffer);
	}

	void VulkanMesh::Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize) {
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		_hash = 0;
		{


			std::vector<uint32_t> vertexLocations;
			Vulkan::VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddVertices(vertSize, pvertices)
				.build(_vertexBuffer, vertexLocations);
			_hash = Core::HashFNV1A(pvertices, vertSize);
			
		}
		{
			std::vector<uint32_t> indexLocations;
			Vulkan::IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddIndices(indSize, pindices)
				.build(_indexBuffer, indexLocations);
			_indexCount = indSize / sizeof(uint32_t);
			//_hash ^= Core::HashFNV1A(pindices, indSize); //necessary to distinguish between meshes? 
		}
	}

	void VulkanMesh::Render()
	{
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;		
		vkCmdDrawIndexed(frameData.cmd, _indexCount, 1, 0, 0, 0);
	}
	void VulkanMesh::Bind()
	{
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;
		vkCmdBindIndexBuffer(frameData.cmd, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(frameData.cmd, 0, 1, &_vertexBuffer.buffer, offsets);
	}
	size_t VulkanMesh::GetHash()
	{
		return _hash;
	}
}
