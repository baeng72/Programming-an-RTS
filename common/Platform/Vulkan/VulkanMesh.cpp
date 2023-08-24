#pragma once
#include "../../Core/defines.h"
#include "VulkanMesh.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
Renderer::Mesh* Renderer::Mesh::Create(RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize) {
	return new Vulkan::VulkanMesh(pdevice, pvertices, vertSize, pindices, indSize);
}
namespace Vulkan {
	
	VulkanMesh::VulkanMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize):_pdevice(pdevice)
	{

		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		{
			std::vector<uint32_t> vertexLocations;
			Vulkan::VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddVertices(vertSize, pvertices)
				.build(_vertexBuffer, vertexLocations);
		}
		{
			std::vector<uint32_t> indexLocations;
			Vulkan::IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddIndices(indSize, pindices)
				.build(_indexBuffer, indexLocations);
			_indexCount = indSize / sizeof(uint32_t);
		}
	}
	VulkanMesh::~VulkanMesh()
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		
		Vulkan::cleanupBuffer(context.device, _vertexBuffer);
		Vulkan::cleanupBuffer(context.device, _indexBuffer);
	}
	void VulkanMesh::Render(Renderer::Shader*pshader)
	{
		pshader->Bind();
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;
		vkCmdBindIndexBuffer(frameData.cmd, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(frameData.cmd, 0, 1, &_vertexBuffer.buffer, offsets);
		vkCmdDrawIndexed(frameData.cmd, _indexCount, 1, 0, 0, 0);
	}
}
